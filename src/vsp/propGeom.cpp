//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    Prop Geometry Class
//
//
//   J.R. Gloudemans - 10/15/03
//
//******************************************************************************

#include "propGeom.h"
#include "af.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "bezier_surf.h"
#include "parmLinkMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"

//==== Constructor =====//
Section::Section()
{
	x_off.initialize(NULL, "Loc_xR", 0.05 );
	x_off.set_lower_upper(0.0, 1.0  );

	y_off.initialize(NULL, "Offset_yR", 0.0 );
	y_off.set_lower_upper(-1.0, 1.0);

	chord.initialize(NULL, "Chord_cR", 0.10 );
	chord.set_lower_upper(0.001, 100.0);

	twist.initialize(NULL, "Twist", 0.0 );
	twist.set_lower_upper(-180.0, 180.0);

}

void Section::SetGeomPtr( Geom* geomPtr )
{
	x_off.set_geom( geomPtr );
	x_off.set_base_geom( geomPtr );
	y_off.set_geom( geomPtr );
	y_off.set_base_geom( geomPtr );
	chord.set_geom( geomPtr );
	chord.set_base_geom( geomPtr );
	twist.set_geom( geomPtr );
	twist.set_base_geom( geomPtr );
}


//==== Constructor =====//
PropGeom::PropGeom(Aircraft* aptr) : Geom(aptr)
{
	type = PROP_GEOM_TYPE;
	type_str = Stringc("prop");

	char name[255];
	sprintf( name, "Prop_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	setSymCode(NO_SYM);

	numXsecs.deactivate();

	numBlades = 4;
	bladeVec.resize( numBlades );

	surfVec.clear();
	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
		surfVec.push_back( &bladeVec[i] );

	diameter.initialize(this, "Diameter", 4.0);
	diameter.set_lower_upper(0.0001, 1000000.0);
	diameter.set_script("prop_shape diameter", 0);

	cone_angle.initialize(this, "Cone_Angle", 0.0 );
	cone_angle.set_lower_upper(-89, 89);
	cone_angle.set_script("prop_shape coneang", 0);

	pitch.initialize(this, "Pitch", 0.0 );
	pitch.set_lower_upper(-180, 180);
	pitch.set_script("prop_shape bladepitch", 0);

	Section sect;
	sect.x_off = 0.05;
	sect.y_off = 0.0;
	sect.chord = 0.08;
	sect.twist = 45.0;
	sect.foil = new Af(this);
	sect.foil->set_num_pnts( (int)numPnts() );
	sect.foil->get_thickness()->set( 0.2 );
	sect.foil->generate_airfoil();
	sect.foil->load_name();
	sect.foil->init_script(Stringc("prop_foil"));
	sectVec.push_back( sect );

	sect.x_off = 0.50;
	sect.y_off = 0.0;
	sect.chord = 0.12;
	sect.twist = 15.0;
	sect.foil = new Af(this);
	sect.foil->set_num_pnts( (int)numPnts() );
	sect.foil->get_thickness()->set( 0.2 );
	sect.foil->generate_airfoil();
	sect.foil->load_name();
	sect.foil->init_script(Stringc("prop_foil"));
	sectVec.push_back( sect );

	sect.x_off = 0.90;
	sect.y_off = 0.0;
	sect.chord = 0.07;
	sect.twist = 5.0;
	sect.foil = new Af(this);
	sect.foil->set_num_pnts( (int)numPnts() );
	sect.foil->get_thickness()->set( 0.2 );
	sect.foil->generate_airfoil();
	sect.foil->load_name();
	sect.foil->init_script(Stringc("prop_foil"));
	sectVec.push_back( sect );

	sect.x_off = 1.00;
	sect.y_off = 0.0;
	sect.chord = 0.02;
	sect.twist = 2.0;
	sect.foil = new Af(this);
	sect.foil->set_num_pnts( (int)numPnts() );
	sect.foil->get_thickness()->set( 0.2 );
	sect.foil->generate_airfoil();
	sect.foil->load_name();
	sect.foil->init_script(Stringc("prop_foil"));
	sectVec.push_back( sect );

	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
		sectVec[i].SetGeomPtr( this );

	currSectID = 0;

	smoothFlag = 1;
	numU = 3;
	numW = 1;
	generate();
}

//==== Destructor =====//
PropGeom::~PropGeom()
{
	DeleteAllFoilsAndSects();
	//for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
	//	delete sectVec[i].foil;
}

void PropGeom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != PROP_GEOM_TYPE )
		return;

	PropGeom* g = (PropGeom*)fromGeom;			// Upcast

	numU = g->getNumU();
	numW = g->getNumW();
	numBlades = g->getNumBlades();
	bladeVec.resize( numBlades );

	surfVec.clear();
	for ( i = 0 ; i < (int)bladeVec.size() ; i++ )
		surfVec.push_back( &bladeVec[i] );

	diameter.set( g->get_diameter()->get() );
	cone_angle.set( g->get_cone_angle()->get() );
	pitch.set( g->get_pitch()->get() );

	//==== Delete Current Foils ====//
	DeleteAllFoilsAndSects();
	//for ( i = 0 ; i < (int)sectVec.size() ; i++ )
	//	delete sectVec[i].foil;

	vector<Section> sVec = g->getSectVec();	

	sectVec = sVec;

	for ( i = 0 ; i < (int)sectVec.size() ; i++ )
	{
		sectVec[i].foil = new Af(this);
		sectVec[i].foil->init_script(Stringc("prop_foil"));
		*(sectVec[i].foil) = *(sVec[i].foil);
		sectVec[i].foil->set_geom( this );
	}

	currSectID = 0;

	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
		sectVec[i].SetGeomPtr( this );

	setCurrSectID( currSectID );
	compose_model_matrix();
	generate();

}

void PropGeom::DeleteAllFoilsAndSects()
{
	//==== Delete Foils && Remove Parm Refs for Sects ====//
	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
	{
		RemoveFoilParmReferences( sectVec[i].foil );
		RemoveSectParmReferences( i );
		delete sectVec[i].foil;
	}
	sectVec.clear();
}


void PropGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &diameter, parmVec, this, "Design" );
	Geom::AddLinkableParm( &cone_angle, parmVec, this, "Design" );
	Geom::AddLinkableParm( &pitch, parmVec, this, "Design" );

	char gname[256];
	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
	{
		sprintf( gname, "Station_%2d", i );

		Geom::AddLinkableParm( sectVec[i].get_x_off(), parmVec, this, gname );
		Geom::AddLinkableParm( sectVec[i].get_y_off(), parmVec, this, gname );
		Geom::AddLinkableParm( sectVec[i].get_chord(), parmVec, this, gname );
		Geom::AddLinkableParm( sectVec[i].get_twist(), parmVec, this, gname );

		sprintf( gname, "Airfoil_%2d", i );
		Af* foil = sectVec[i].foil;
		vector< Parm* > pVec = foil->GetLinkableParms();
		for ( int j = 0 ; j < (int)pVec.size() ; j++ )
		{
			Geom::AddLinkableParm( pVec[j], parmVec, this, gname );
		}
	}
}

void PropGeom::RemoveFoilParmReferences( Af* foil )
{
	vector< Parm* > pVec = foil->GetLinkableParms();
	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		parmMgrPtr->RemoveParmReferences( pVec[i] );
	}
}

void PropGeom::RemoveSectParmReferences( int sect_id )
{
	if ( sect_id < 0 || sect_id >= (int)sectVec.size() )
		return;

	parmMgrPtr->RemoveParmReferences( &sectVec[sect_id].x_off );
	parmMgrPtr->RemoveParmReferences( &sectVec[sect_id].y_off );
	parmMgrPtr->RemoveParmReferences( &sectVec[sect_id].chord );
	parmMgrPtr->RemoveParmReferences( &sectVec[sect_id].twist );
}

void PropGeom::addStation()
{
	int i;
	int id = currSectID;
	if ( id >= (int)sectVec.size()-1 )			// Dont add past last station
		return;

	Section sect = sectVec[id];
	sect.x_off = 0.5*(sectVec[id].x_off() + sectVec[id+1].x_off() );
	sect.y_off = 0.5*(sectVec[id].y_off() + sectVec[id+1].y_off() );
	sect.chord = 0.5*(sectVec[id].chord() + sectVec[id+1].chord() );
	sect.twist = 0.5*(sectVec[id].twist() + sectVec[id+1].twist() );
	sect.foil = new Af(this);
	*(sect.foil) = *(sectVec[id].foil);
	sect.foil->set_geom( this );
	sect.foil->init_script(Stringc("prop_foil"));
	
	vector< Section > tempVec;
	for ( i = 0 ; i <= id ; i++ )
		tempVec.push_back( sectVec[i] );

	tempVec.push_back( sect );

	for ( i = id+1 ; i < (int)sectVec.size() ; i++)
		tempVec.push_back( sectVec[i] );

	sectVec = tempVec;

	setCurrSectID( currSectID+1 );
	generate();

	parmMgrPtr->RebuildAll();

}

void PropGeom::delStation()
{
	if ( sectVec.size() <= 2 )				// Need at least 2 stations
		return;

	if ( currSectID >= (int)sectVec.size()-1 )	// Dont delete last station
		return;

	RemoveSectParmReferences(currSectID);

	Af* foilToDel = sectVec[currSectID].foil;

	vector< Section > tempVec;
	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
	{
		if ( currSectID != i  )
			tempVec.push_back( sectVec[i] );
	}

	sectVec = tempVec;

	RemoveFoilParmReferences ( foilToDel );
	delete foilToDel;

	setCurrSectID( currSectID );
	generate();

	parmMgrPtr->RebuildAll();


}

void PropGeom::setNumBlades(int b)
{
	if ( b == numBlades )
		return;

	numBlades = b;
	bladeVec.resize( numBlades );

	surfVec.clear();
	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
		surfVec.push_back( &bladeVec[i] );

	generate();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );

}

void PropGeom::setCurrSectID( int id )
{
	if ( id < 0 ) id = 0;
	else if ( id > (int)sectVec.size()-1 ) id = sectVec.size()-1;

	currSectID = id;

	//==== Restrict Loc of Section Between 2 Adjoining Sections ====//
	if ( currSectID == 0 )
		get_loc()->set_lower_upper(0.0, sectVec[currSectID+1].x_off() );
	else if ( currSectID == sectVec.size()-1 )
		get_loc()->set_lower_upper(1.0, 1.0 );
	else
		get_loc()->set_lower_upper(sectVec[currSectID-1].x_off(), sectVec[currSectID+1].x_off() );


	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );



}

void PropGeom::setSmoothFlag( int f )
{
	smoothFlag = f;
	generate();
	airPtr->geomMod( this );
}

void PropGeom::setNumU( int n )
{
	if ( n < 1 ) n = 1;
	numU = n;
	generate();
	airPtr->geomMod( this );
}

void PropGeom::setNumW( int n )
{
	if ( n < 1 ) n = 1;
	numW = n;
	generate();
	airPtr->geomMod( this );
}

//==== Generate Fuse Component ====//
void PropGeom::generate()
{
	int i, j;

	Xsec_surf surf;
	surf.set_num_pnts( numPnts.iget() );
	surf.set_num_xsecs( sectVec.size()+2 );

	//==== Load Up Airfoil ====//
	for ( int ip = 0 ; ip < surf.get_num_pnts() ; ip++ )
		surf.set_pnt(0, ip, sectVec[0].foil->get_end_cap(ip) );	

	int numxs = sectVec.size();
	for ( i = 0 ; i < numxs ; i++ )
	{
		for ( j = 0 ; j < surf.get_num_pnts() ; j++ )
		{
			surf.set_pnt( i+1, j, sectVec[i].foil->get_pnt(j) );
		}
	}
	for ( int ip = 0 ; ip < surf.get_num_pnts() ; ip++ )
		surf.set_pnt(numxs+1, ip, sectVec[numxs-1].foil->get_end_cap(ip) );	

	//==== Build Up One Blade ====//
	double rad = diameter()/2.0;
	for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
	{
		int sid = i;

		if ( i > 0 ) sid = i-1;

		if ( i > (int)sectVec.size()-1 ) sid = (int)sectVec.size()-1;

		Section* sPtr = &(sectVec[sid]);
		surf.scale_xsec_x(  i, sPtr->chord()*rad );
		surf.scale_xsec_z(  i, sPtr->chord()*rad );
		surf.offset_xsec_x( i, -0.5*(sPtr->chord()*rad) );
		surf.rotate_xsec_y( i, 90.0 );
		surf.rotate_xsec_y( i, -sPtr->twist() - pitch() );
		surf.offset_xsec_y( i, sPtr->x_off()*rad );
		surf.offset_xsec_z( i, -sPtr->y_off()*rad );
	}

	//==== Set Flags So Trailing Edge Remains Sharp ====//
	surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
	surf.set_pnt_tan_flag( 1, Bezier_curve::SHARP );
    int num_pnts  = surf.get_num_pnts();
	surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	if ( !smoothFlag )
	{
		for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
		{
			surf.set_xsec_tan_flag( i, Bezier_curve::SHARP );
		}
	}
	else	// Sharpen End Caps
	{
		surf.set_xsec_tan_flag( surf.get_num_xsecs()-2, Bezier_curve::SHARP );
	}


	bezier_surf besurf;
	surf.load_bezier_surface( &besurf );
	int umax = besurf.get_u_max();
	int wmax = besurf.get_w_max();

	Xsec_surf smooth_surf;
	smooth_surf.set_num_pnts( (surf.get_num_pnts()-1)*numW + 1 );
	smooth_surf.set_num_xsecs( (surf.get_num_xsecs()-1)*numU + 1 );

	for ( i = 0 ; i < smooth_surf.get_num_xsecs() ; i++ )
	{
		double fu = (double)i/(double)(smooth_surf.get_num_xsecs()-1);
		double u  = fu*(double)umax;
		for ( j = 0 ; j < smooth_surf.get_num_pnts() ; j++ )
		{
			double fw = (double)j/(double)(smooth_surf.get_num_pnts()-1);
			double w  = fw*(double)wmax;
			vec3d p = besurf.comp_pnt( u, w );
			smooth_surf.set_pnt( i, j, p );
		}
	}

	//==== Load Blades into bladeVec ====//
	for ( int nb = 0 ; nb < (int)bladeVec.size() ; nb++ )
	{
		bladeVec[nb].set_num_pnts( smooth_surf.get_num_pnts() );
		bladeVec[nb].set_num_xsecs(  smooth_surf.get_num_xsecs() );

		//==== Load Points ====//
		for ( i = 0 ; i < smooth_surf.get_num_xsecs() ; i++ )
			for ( j = 0 ; j < smooth_surf.get_num_pnts() ; j++ )
				bladeVec[nb].set_pnt( i, j, smooth_surf.get_pnt( i, j ) );

		double xang = 360.0*(double)nb/(double)(bladeVec.size());

		for ( i = 0 ; i < bladeVec[nb].get_num_xsecs() ; i++ )
		{
			bladeVec[nb].rotate_xsec_z( i, cone_angle() );
			bladeVec[nb].rotate_xsec_x( i, xang );
		}
		bladeVec[nb].load_refl_pnts_xsecs();
		bladeVec[nb].load_hidden_surf();
		bladeVec[nb].load_normals();
		bladeVec[nb].load_uw();

	}

	for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
		sectVec[i].SetGeomPtr( this );

	update_bbox();


}


void PropGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( 0.0 ); 
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void PropGeom::parm_changed(Parm* p)
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
		numPnts  = 4*((int)numPnts()/4)+1;

		for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
		{
			sectVec[i].foil->set_num_pnts( numPnts.iget() );
		}
	}
	else if ( p->get_update_grp() == AF_UPDATE_GROUP )
	{
		for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
		{
			sectVec[i].foil-> generate_airfoil();
			sectVec[i].foil-> load_name();
		}
		//get_af_ptr()->generate_airfoil();
		//get_af_ptr()->load_name();
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



//==== Write Fuse File ====//
void PropGeom::write(FILE* file_id)
{
}

//==== Write External File ====//
void PropGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Prop");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr prop_node = xmlNewChild( root, NULL, (const xmlChar *)"Prop_Parms", NULL );

  xmlAddIntNode( prop_node, "NumBlades", numBlades );
  xmlAddIntNode( prop_node, "SmoothFlag", smoothFlag );
  xmlAddIntNode( prop_node, "NumU", numU );
  xmlAddIntNode( prop_node, "NumW", numW );

  xmlAddDoubleNode( prop_node, "Diameter", diameter() );
  xmlAddDoubleNode( prop_node, "ConeAngle", cone_angle() );
  xmlAddDoubleNode( prop_node, "Pitch", pitch() );

  for ( int i = 0 ; i < (int)sectVec.size() ; i++ )
  {
	xmlNodePtr sect_node = xmlNewChild( prop_node, NULL, (const xmlChar *)"Sect_Parms", NULL );

	xmlAddDoubleNode( sect_node, "X_Off", sectVec[i].x_off() );
	xmlAddDoubleNode( sect_node, "Y_Off", sectVec[i].y_off() );
	xmlAddDoubleNode( sect_node, "Chord", sectVec[i].chord() );
	xmlAddDoubleNode( sect_node, "Twist", sectVec[i].twist() );

    xmlNodePtr af_node = xmlNewChild( sect_node, NULL, (const xmlChar *)"Airfoil", NULL );
    sectVec[i].foil->write( af_node );
  }

}

//==== Read External File ====//
void PropGeom::read(xmlNodePtr root)
{
  int i;

  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Prop_Parms", 0 );
  if ( node )
  {
	numBlades   =  xmlFindInt( node, "NumBlades", numBlades );
	smoothFlag  =  xmlFindInt( node, "SmoothFlag", smoothFlag );
	numU        =  xmlFindInt( node, "NumU", numU );
	numW        =  xmlFindInt( node, "NumW", numW );

	diameter    =  xmlFindDouble( node, "Diameter", diameter() );
	cone_angle  =  xmlFindDouble( node, "ConeAngle", diameter() );
	pitch       =  xmlFindDouble( node, "Pitch", diameter() );

	int num_sects = xmlGetNumNames( node, "Sect_Parms" );

	if ( num_sects >= 2 )
	{
		//==== Delete Old Sect Vec ====//
		DeleteAllFoilsAndSects();

		//for ( i = 0 ; i < (int)sectVec.size() ; i++ )
		//{
		//	RemoveFoilParmReferences( sectVec[i].foil );
		//	delete sectVec[i].foil;
		//}

		sectVec.resize( num_sects );

		for ( i = 0 ; i < (int)sectVec.size() ; i++ )
		{
			xmlNodePtr sect_node = xmlGetNode( node, "Sect_Parms", i );

			sectVec[i].chord = xmlFindDouble( sect_node, "Chord", sectVec[i].chord() );
			sectVec[i].x_off = xmlFindDouble( sect_node, "X_Off", sectVec[i].x_off() );
			sectVec[i].y_off = xmlFindDouble( sect_node, "Y_Off", sectVec[i].y_off() );
			sectVec[i].twist = xmlFindDouble( sect_node, "Twist", sectVec[i].twist() );

			sectVec[i].foil = new Af(this);
			sectVec[i].foil->init_script(Stringc("prop_foil"));

			xmlNodePtr af_node = xmlGetNode( sect_node, "Airfoil", 0 );
			if ( af_node )
				sectVec[i].foil->read( af_node);
			sectVec[i].foil->set_num_pnts( numPnts.iget() );
			sectVec[i].foil->generate_airfoil();
			sectVec[i].foil->load_name();
		}
	}
  }

  currSectID = 0;

  generate();
  parmMgrPtr->RebuildAll();

}



//==== Read Fuse File ====//
void PropGeom::read(FILE* file_id)
{
}

//==== Generate Cross Sections =====//
void PropGeom::regenerate()
{
}

//==== Write Rhino File ====//
void PropGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	int save_num_u = numU;
	numU = 1;
	generate();

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		bladeVec[i].load_refl_pnts_xsecs();
		bladeVec[i].clear_pnt_tan_flags();
		bladeVec[i].clear_xsec_tan_flags();

		//==== Sharpen Trailing Edge of Wing ====//
		bladeVec[i].set_pnt_tan_flag( 0, Bezier_curve::SHARP );
		int num_pnts  = bladeVec[i].get_num_pnts();
		bladeVec[i].set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

		//==== Sharpen Wing Joints ====//
		int num_xsecs = bladeVec[i].get_num_xsecs();
		bladeVec[i].set_xsec_tan_flag( 0, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( 1, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( num_xsecs-1, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );

		//==== Set Splits ====//
		vector <int> u_split;
		u_split.push_back( 0 );
		u_split.push_back( 3 );
		u_split.push_back( 3*(bladeVec[i].get_num_xsecs() - 2) );
		u_split.push_back( 3*(bladeVec[i].get_num_xsecs() - 1) );

		vector <int> w_split;
		w_split.push_back( 0 );
		w_split.push_back( 3*(bladeVec[i].get_num_pnts()/2)  );
		w_split.push_back( 3*(bladeVec[i].get_num_pnts() - 1)  );

		//==== Write File ====//
		bladeVec[i].write_split_rhino_file( sym_code, model_mat, reflect_mat, 
			u_split, w_split, false, archive, attributes );
	}

	numU = save_num_u;
	generate();

}

int  PropGeom::get_num_bezier_comps()
{
	int num_comps = bladeVec.size();
	if ( sym_code != NO_SYM )
		num_comps *= 2;
	return num_comps;
}

//==== Return Number of Xsec Surfs to Write ====//
int PropGeom::getNumXSecSurfs()
{
	if ( !outputFlag )
		return 0;

	if ( sym_code == NO_SYM )
		return bladeVec.size();
	else
		return 2*bladeVec.size();
}

//==== Dump Xsec File =====//
void PropGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
			printf("%f\t ", model_mat[i][i]);
		printf("\n");
	}

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		//==== Only Write Out OML not IML ====//
		fprintf(dump_file, "\n");
		fprintf(dump_file, "%s \n",(char*) getName());
		fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
		fprintf(dump_file, " TYPE              = 1\n");
		fprintf(dump_file, " CROSS SECTIONS    = %d \n", bladeVec[i].get_num_xsecs() );
		fprintf(dump_file, " PTS/CROSS SECTION = %d \n", bladeVec[i].get_num_pnts() );

		// Write out cross sections
		for ( int j = 0 ; j < bladeVec[i].get_num_xsecs() ; j++ )
		{
			bladeVec[i].write_xsec(j, model_mat, dump_file);
		}
	}

	if ( sym_code == NO_SYM ) return;

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		//==== Only Write Out OML not IML ====//
		fprintf(dump_file, "\n");
		fprintf(dump_file, "%s \n",(char*) getName());
		fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
		fprintf(dump_file, " TYPE              = 1\n");
		fprintf(dump_file, " CROSS SECTIONS    = %d \n", bladeVec[i].get_num_xsecs() );
		fprintf(dump_file, " PTS/CROSS SECTION = %d \n", bladeVec[i].get_num_pnts() );

		// Write out cross sections
		for ( int j = 0 ; j < bladeVec[i].get_num_xsecs() ; j++ )
		{
			bladeVec[i].write_refl_xsec( sym_code, j, reflect_mat, dump_file );
		}
	}
}

void PropGeom::write_bezier_file( int id, FILE* file_id )
{
	int save_num_u = numU;
	numU = 1;
	generate();

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		bladeVec[i].load_refl_pnts_xsecs();
		bladeVec[i].clear_pnt_tan_flags();
		bladeVec[i].clear_xsec_tan_flags();

		//==== Sharpen Trailing Edge of Wing ====//
		bladeVec[i].set_pnt_tan_flag( 0, Bezier_curve::SHARP );
		int num_pnts  = bladeVec[i].get_num_pnts();
		bladeVec[i].set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

		//==== Sharpen Wing Joints ====//
		int num_xsecs = bladeVec[i].get_num_xsecs();
		bladeVec[i].set_xsec_tan_flag( 0, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( 1, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( num_xsecs-1, Bezier_curve::SHARP );
		bladeVec[i].set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );

		//==== Set Splits ====//
		vector <int> u_split;
		u_split.push_back( 0 );
		u_split.push_back( 3 );
		u_split.push_back( 3*(bladeVec[i].get_num_xsecs() - 2) );
		u_split.push_back( 3*(bladeVec[i].get_num_xsecs() - 1) );

		vector <int> w_split;
		w_split.push_back( 0 );
		w_split.push_back( 3*(bladeVec[i].get_num_pnts()/2)  );
		w_split.push_back( 3*(bladeVec[i].get_num_pnts() - 1)  );

		bladeVec[i].write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
	}

	numU = save_num_u;
	generate();

}





//==== Convert To Tri Mesh ====//
vector< TMesh* > PropGeom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		tMeshVec.push_back( bladeVec[i].createTMesh(model_mat) );
	}

	if ( sym_code != NO_SYM )
	{
		for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
		{
			tMeshVec.push_back( bladeVec[i].createReflTMesh(sym_code, reflect_mat) );
		}
	}
	return tMeshVec;
}


//==== Generate Cross Sections =====//
void PropGeom::update_bbox()
{

  vec3d tmp_pnt;
  bbox new_box;

  for ( int nb = 0 ; nb < (int)bladeVec.size() ; nb++ )
  {
	int num_pnts  = bladeVec[nb].get_num_pnts();
	int num_xsecs = bladeVec[nb].get_num_xsecs();

	  //==== Check All Xsecs ====//
	  for ( int i = 0 ; i < num_xsecs ; i++ )
		for ( int j = 0 ; j < num_pnts ; j+=2 )
		{
			new_box.update(bladeVec[nb].get_pnt(i,j));
		}
  }

  bnd_box = new_box;

  update_xformed_bbox();			// Load Xform BBox

}


void PropGeom::draw()
{
	Geom::draw();

	int nb;

	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Set Up Xsec Hightlighting ====//
	if ( redFlag )
	{
		for ( nb = 0 ; nb < (int)bladeVec.size() ; nb++ )
		{
			bladeVec[nb].set_highlight_xsec_flag(1);
			bladeVec[nb].set_highlight_xsec_id( (currSectID+1)*numU );	
		}
	}
	else
	{
		for ( nb = 0 ; nb < (int)bladeVec.size() ; nb++ )
			bladeVec[nb].set_highlight_xsec_flag(0);
	}


}

//==== Draw If Alpha < 1 and Shaded ====//
void PropGeom::drawAlpha()
{
	Geom::drawAlpha();
}

//==== Compute And Load Normals ====//
void PropGeom::load_normals()
{
}

//==== Draw Other Pnts XSecs====//
void PropGeom::load_hidden_surf()
{
}


void PropGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	diameter.set( diameter()*current_factor );

	lastScaleFactor = scaleFactor();

}


void PropGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);

}

void PropGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;

	generate();
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

vec3d PropGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;
//jrg FIX
	vec3d uvpnt = vec3d(0,0,0);
	pos = uvpnt.transform( model_mat );

	return pos;
}

vec3d PropGeom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	return bladeVec[surfid].get_vertex(x, p, r); 
}
void  PropGeom::getVertexVec(vector< VertexID > *vertVec)
{ 
	for (int i = 0; i < (int)bladeVec.size(); i++) 
		buildVertexVec(&bladeVec[i], i, vertVec); 
}

void PropGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	vector< vec3d > intPntVec;

	for ( int i = 0 ; i < (int)bladeVec.size() ; i++ )
	{
		int np = bladeVec[i].get_num_pnts();
		int nxs = bladeVec[i].get_num_xsecs();
		vec3d p0 = bladeVec[i].get_pnt( nxs/2, np/4 );
		vec3d p1 = bladeVec[i].get_pnt( nxs/2, 3*np/4 );
		intPntVec.push_back( (p0 + p1)*0.5 );
		vec3d tp = intPntVec.back().transform( model_mat );
		pVec.push_back( tp );
	}

	if (sym_code != NO_SYM)
	{
		for ( int i = 0 ; i < (int)intPntVec.size() ; i++ )
		{
			vec3d tp = (intPntVec[i] * sym_vec).transform(reflect_mat);
			pVec.push_back( tp );
		}
    }
}


