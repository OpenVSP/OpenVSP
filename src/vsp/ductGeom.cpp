//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//    Duct  Geometry Class
//
//
//   J.R. Gloudemans - 10/15/03
//
//******************************************************************************

#include "ductGeom.h"
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
DuctGeom::DuctGeom(Aircraft* aptr) : Geom(aptr)
{
	type = DUCT_GEOM_TYPE;
	type_str = Stringc("duct");

	char name[255];
	sprintf( name, "Duct_%d", geomCnt ); 
	geomCnt++;
	name_str = Stringc(name);
	id_str = name_str;			
	setSymCode(NO_SYM);

	foil = new Af(this);
	foil->set_num_pnts( numPnts.iget() );

	foil->init_script(Stringc("duct_foil"));
	/*
	foil->get_camber()->set_script("duct_foil camber", 0);
	foil->get_camber_loc()->set_script("duct_foil camberloc", 0);
	foil->get_thickness()->set_script("duct_foil thick", 0);
	foil->get_thickness_loc()->set_script("duct_foil thickloc", 0);
	foil->get_ideal_cl()->set_script("duct_foil idealcl", 0);
	foil->get_a()->set_script("duct_foil a", 0);
	*/

	length.initialize(this, "Length", 5.0);
	length.set_lower_upper(0.00001, 1000000.0);
	length.set_script("duct_shape length", 0);

	chord.initialize(this, "Chord", 5.0);
	chord.set_lower_upper(0.00001, 1000000.0);
	chord.deactivate();

	inlet_dia.initialize(this, "Inlet Diameter", 10.0);
	inlet_dia.set_lower_upper(0.00001, 1000000.0);
	inlet_dia.set_script("duct_shape inletdia", 0);

	inlet_area.initialize(this, "Inlet Area", 10.0 );
	inlet_area.set_lower_upper(0.00001, 1000000.0);
	inlet_area.deactivate();

	outlet_dia.initialize(this, "Outlet Diameter", 10.0);
	outlet_dia.set_lower_upper(0.00001, 1000000.0);
	outlet_dia.deactivate();

	outlet_area.initialize(this, "Outlet Area", 10.0 );
	outlet_area.set_lower_upper(0.00001, 1000000.0);
	outlet_area.deactivate();

	inlet_outlet.initialize(this, "Inlet Outlet Ratio", 1.0);
	inlet_outlet.set_lower_upper(0.01, 100.0);
	inlet_outlet.set_script("duct_shape inletoutlet", 0);

	numXsecs = 41;
	surf.set_num_pnts( numPnts.iget() );
	surf.set_num_xsecs(  numXsecs.iget() );

	generate();

	surfVec.push_back( &surf );


}

//==== Destructor =====//
DuctGeom::~DuctGeom()
{
	delete foil;
}

void DuctGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != DUCT_GEOM_TYPE )
		return;

	DuctGeom* g = (DuctGeom*)fromGeom;			// UpCast

	length.set( g->get_length()->get() );
	chord.set( g->get_chord()->get() );
	inlet_dia.set( g->get_inlet_dia()->get() );
	inlet_area.set( g->get_inlet_area()->get() );
	outlet_dia.set( g->get_outlet_dia()->get() );
	outlet_area.set( g->get_outlet_area()->get() );
	inlet_outlet.set( g->get_inlet_outlet()->get() );

	*foil = *(g->get_af_ptr());
	foil->set_geom( this );

	foil->set_num_pnts( numPnts.iget() );

	surf.set_num_pnts( numPnts.iget() );
	surf.set_num_xsecs(  numXsecs.iget() );

	compose_model_matrix();
	generate();

}

void DuctGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );
	Geom::AddLinkableParm( &chord, parmVec, this, "Design" );
	Geom::AddLinkableParm( &inlet_dia, parmVec, this, "Design" );
	Geom::AddLinkableParm( &inlet_area, parmVec, this, "Design" );
	Geom::AddLinkableParm( &outlet_dia, parmVec, this, "Design" );
	Geom::AddLinkableParm( &outlet_area, parmVec, this, "Design" );
	Geom::AddLinkableParm( &inlet_outlet, parmVec, this, "Design" );

	vector< Parm* > pVec = foil->GetLinkableParms();
	for ( int i = 0 ; i < (int)pVec.size() ; i++ )
	{
		Geom::AddLinkableParm( pVec[i], parmVec, this, "Airfoil" );
	}
}

//==== Generate Fuse Component ====//
void DuctGeom::generate()
{
	int i, j;

	//==== Load Up Airfoil ====//
	for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
	{
		for ( j = 0 ; j < surf.get_num_pnts() ; j++ )
		{
			surf.set_pnt( i, j, foil->get_pnt(j) );
		}
	}

	//==== Compute Dia and Angle ====//
	inlet_area = PI*0.25*inlet_dia()*inlet_dia();

	outlet_dia = inlet_dia()/inlet_outlet();
	outlet_area = PI*0.25*outlet_dia()*outlet_dia();

	double delr = outlet_dia()/2.0 - inlet_dia()/2.0;

	chord = sqrt(delr*delr + length()*length());

	double alpha = atan( delr/length() );


	//==== Move and Rotate XSec Section ====//
	for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
	{
		double xsectFract = (double)i/(double)(surf.get_num_xsecs()-1);

		surf.scale_xsec_x(  i, chord() );
		surf.scale_xsec_z(  i, chord() );
		surf.rotate_xsec_y( i, alpha*RAD_2_DEG );
		surf.offset_xsec_z( i, inlet_dia()/2.0 );
		surf.rotate_xsec_x( i, xsectFract*360.0 );
	}

	surf.copy_xsec( &surf, 0, surf.get_num_xsecs()-1 );

	surf.load_refl_pnts_xsecs();
    surf.load_hidden_surf();
    surf.load_normals();
	surf.load_uw();

	update_bbox();

}


void DuctGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*chord()*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void DuctGeom::parm_changed(Parm* p)
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
		numPnts  = 4*(numPnts.iget()/4)+1;
		numXsecs = 4*(numXsecs.iget()/4)+1;

		foil->set_num_pnts( numPnts.iget() );
		surf.set_num_pnts( numPnts.iget() );
		surf.set_num_xsecs(  numXsecs.iget() );

	}
	else if ( p->get_update_grp() == AF_UPDATE_GROUP )
	{
		get_af_ptr()->generate_airfoil();
		get_af_ptr()->load_name();
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
void DuctGeom::write(FILE* file_id)
{
}

//==== Write External File ====//
void DuctGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Duct");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr duct_node = xmlNewChild( root, NULL, (const xmlChar *)"Duct_Parms", NULL );

  xmlAddDoubleNode( duct_node, "Length", length() );
  xmlAddDoubleNode( duct_node, "Chord",  chord() );
  xmlAddDoubleNode( duct_node, "Inlet_Dia", inlet_dia() );
  xmlAddDoubleNode( duct_node, "Inlet_Area", inlet_area() );
  xmlAddDoubleNode( duct_node, "Outlet_Dia", outlet_dia() );
  xmlAddDoubleNode( duct_node, "Outlet_Area", outlet_area() );
  xmlAddDoubleNode( duct_node, "Inlet_Outlet", inlet_outlet() );

  //==== Write Airfoil ====//
  xmlNodePtr af_node = xmlNewChild( root, NULL, (const xmlChar *)"Airfoil", NULL );
  foil->write( af_node );


}

//==== Read External File ====//
void DuctGeom::read(xmlNodePtr root)
{

  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Duct_Parms", 0 );
  if ( node )
  {
    length       =  xmlFindDouble( node, "Length", length() );
    chord        =  xmlFindDouble( node, "Chord", chord() );
    inlet_dia    =  xmlFindDouble( node, "Inlet_Dia", inlet_dia() );
    inlet_area   =  xmlFindDouble( node, "Inlet_Area", inlet_area() );
    outlet_dia   =  xmlFindDouble( node, "Outlet_Dia", outlet_dia() );
    outlet_area  =  xmlFindDouble( node, "Outlet_Area", outlet_area() );
    inlet_outlet =  xmlFindDouble( node, "Inlet_Outlet", inlet_outlet() );
  }

  xmlNodePtr af_node = xmlGetNode( root, "Airfoil", 0 );
  if ( af_node )
    foil->read( af_node);

  generate();

}



//==== Read Fuse File ====//
void DuctGeom::read(FILE* file_id)
{
}

//==== Generate Cross Sections =====//
void DuctGeom::regenerate()
{
}

//==== Write Rhino File ====//
void DuctGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	surf.clear_pnt_tan_flags();
	surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = surf.get_num_pnts();
	surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );
	surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}


int  DuctGeom::get_num_bezier_comps()
{
	int num_comps = 1;
	if ( sym_code != NO_SYM )
		num_comps *= 2;
	return num_comps;
}

void DuctGeom::write_bezier_file( int id, FILE* file_id )
{
	surf.clear_pnt_tan_flags();
	surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = surf.get_num_pnts();
	surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	//==== Set Up Split Values ====//
	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3*(surf.get_num_xsecs()/2) );
	u_split.push_back( 3*(surf.get_num_xsecs() - 1) );

	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(surf.get_num_pnts() - 1)  );

	surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split );
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > DuctGeom:: createTMeshVec()
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

//==== Generate Cross Sections =====//
void DuctGeom::update_bbox()
{

  vec3d tmp_pnt;
  bbox new_box;

  int num_pnts  = surf.get_num_pnts();
  int num_xsecs = surf.get_num_xsecs();

  //==== Check All Xsecs ====//
  for ( int i = 0 ; i < num_xsecs ; i+=4 )
	for ( int j = 0 ; j < num_pnts ; j+=2 )
	{
		vec3d p = surf.get_pnt(i,j);
		new_box.update(p);
	}


  bnd_box = new_box;

  update_xformed_bbox();			// Load Xform BBox

}


void DuctGeom::draw()
{
	Geom::draw();

/*
	if ( fastDrawFlag )
		surf.fast_draw_on();
	else
		surf.fast_draw_off();

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
		surf.draw_wire();
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		surf.draw_refl_wire(sym_code);
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
				surf.draw_shaded();
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
				surf.draw_refl_shaded( sym_code);
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
		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		surf.draw_refl_hidden(sym_code);
		glPopMatrix();		
	}
*/
}

//==== Draw If Alpha < 1 and Shaded ====//
void DuctGeom::drawAlpha()
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
		surf.draw_shaded();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		mat->bind();
		surf.draw_refl_shaded( sym_code );

		glPopMatrix();
	}
*/
}

//==== Compute And Load Normals ====//
void DuctGeom::load_normals()
{
}

//==== Draw Other Pnts XSecs====//
void DuctGeom::load_hidden_surf()
{
}


void DuctGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	length.set( length()*current_factor );
	inlet_dia.set( inlet_dia()*current_factor );

	lastScaleFactor = scaleFactor();

}


void DuctGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);

}

void DuctGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;

	generate();
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

vec3d DuctGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}



//==== Dump Cross Section File ====//
void DuctGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
  int i;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName() );
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");					// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",surf.get_num_pnts());

  for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
    {
      surf.write_xsec(i, model_mat, dump_file );
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");					// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",surf.get_num_pnts());

  for ( i = 0 ; i < surf.get_num_xsecs() ; i++ )
    {
      surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }

}



//==== Return Number Of Felisa Surface Written Out ====//
int DuctGeom::get_num_felisa_comps()
{
  //==== Write out one side top/bot + endcap ====//
  int nxs = surf.get_num_xsecs();

  vec3d v = getTotalTranVec();

  if ( v.y() > 0.0 )
    return( 2*(nxs-1) );
  else
    return( 2*((nxs-1)/2));
}

//==== Return Number Of Felisa Surface Written Out ====//
int DuctGeom::get_num_felisa_wings()
{
  return(0);
}

//==== Write Out Felisa Surfaces To File ====//
int DuctGeom::write_felisa_file(int geom_no, FILE* dump_file)
{
  int isec, j;
  vec3d temp;
  int num_geoms = geom_no;

  vec3d v = getTotalTranVec();

  int nxsecs = get_num_felisa_comps()/2;
  int npnts  = surf.get_num_pnts()/2 + 1;

  //==== Write Lower Surfaces ====//
  for ( isec = 0 ; isec < nxsecs ; isec++ )
	{
      fprintf(dump_file, "\n");
      fprintf(dump_file, "%d                    Surface_Number\n", num_geoms);
      num_geoms++;
      fprintf(dump_file, "%s-Lower-%d           Name\n", (char*) this->getName(),isec);

	  double sdist = dist( surf.get_pnt(isec,0), surf.get_pnt(isec+1,0) );
	  double cdist = dist( surf.get_pnt(isec,0), surf.get_pnt(isec,npnts) );
      int num_slices = (int)(npnts*( sdist/cdist ) );
      num_slices = MAX(num_slices,20);
      fprintf(dump_file, "%d                    U_Render\n",num_slices);
      fprintf(dump_file, "2                     W_Render\n");
      fprintf(dump_file, "2                     Num_xsecs\n");
      fprintf(dump_file, "%d                    Num_pnts\n", npnts);

      for ( j = npnts-1 ; j < surf.get_num_pnts() ; j++ )
        {
          temp = surf.get_pnt( isec, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
      for ( j = npnts-1 ; j < surf.get_num_pnts() ; j++ )
        {
          temp = surf.get_pnt( isec+1, j).transform(model_mat);
          fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
        }
	}

  for (  isec = 0 ; isec < nxsecs ; isec++ )
    {
        //==== Write Upper Wing Surfaces ====//
        fprintf(dump_file, "\n");
        fprintf(dump_file, "%d                        Surface_Number\n", num_geoms);
        num_geoms++;
        fprintf(dump_file, "%s-Upper-%d               Name\n", (char*) this->getName(),isec);
		double sdist = dist( surf.get_pnt(isec,0), surf.get_pnt(isec+1,0) );
		double cdist = dist( surf.get_pnt(isec,0), surf.get_pnt(isec,npnts) );
        int num_slices = (int)(npnts*( sdist/cdist ) );
        num_slices = MAX(num_slices,20);
        fprintf(dump_file, "%d                U_Render\n",num_slices);
        fprintf(dump_file, "2                 W_Render\n");
        fprintf(dump_file, "2                 Num_xsecs\n");
        fprintf(dump_file, "%d                Num_pnts\n", npnts);

          for ( j = 0 ; j < npnts ; j++ )
            {
              temp = surf.get_pnt( isec, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
          for ( j = 0 ; j < npnts ; j++ )
            {
              temp = surf.get_pnt( isec+1, j).transform(model_mat);
              fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", temp.x(), temp.y(),temp.z());
            }
    }

 return(num_geoms);
}

//==== Write Out Felisa Surfaces To File ====//
void DuctGeom::write_bco_info(FILE* file_ptr, int& surf_cnt, int& wing_cnt)
{
  int ixs;
  int nxsecs = get_num_felisa_comps()/2;

  for ( ixs = 0 ; ixs < nxsecs ; ixs++ )
  {
     fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Lower Surf\n", surf_cnt, (char*)getName() );
     surf_cnt++;
  }
  for ( ixs = 0 ; ixs < nxsecs ; ixs++ )
  {
     fprintf( file_ptr, "%d        0      0     0.0     0.0     0.0     0.0   %s Upper Surf\n", surf_cnt, (char*)getName() );
     surf_cnt++;
  }
}


//==== Write Out Felisa Background Lines File ====//
void DuctGeom::write_felisa_line_sources(FILE* dump_file)
{

  int ix;
  int nxsecs = get_num_felisa_comps()/2;
  int half_ind = surf.get_num_pnts()/2;

  //==== Write Out Line Along Trailing Edge ====//
  for ( ix = 0 ; ix < nxsecs ;  ix++ )
  {
	vec3d te_pnt1 = surf.get_pnt(ix,0).transform(model_mat);
	vec3d te_pnt2 = surf.get_pnt(ix+1,0).transform(model_mat);

	vec3d le_pnt1 = surf.get_pnt(ix,half_ind).transform(model_mat);
	vec3d le_pnt2 = surf.get_pnt(ix+1,half_ind).transform(model_mat);

	double chord1 = dist( te_pnt1, le_pnt1 );
	double chord2 = dist( te_pnt2, le_pnt2 );

  	double ds_root_te = chord()/30.;
  	double ds1 = MAX(chord1/chord(),0.50)*ds_root_te;
  	double ds2 = MAX(chord2/chord(),0.50)*ds_root_te;

  	ds1 = MAX(ds1, inlet_dia()*3/1000.);
	ds2 = MAX(ds2, inlet_dia()*3/1000.);

  	//==== Write Out Line Along Trailing Edge ====//
  	fprintf(dump_file, " %s: Trailing Edge Line %d \n", (char*) getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt1.x(), te_pnt1.y(), te_pnt1.z(), ds1, 2.*ds1 ,9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      te_pnt2.x(), te_pnt2.y(), te_pnt2.z(), ds2, 2.*ds2 ,9.*ds2);
  }

  //==== Write Out Line Along Leading Edge ====//
  for ( ix = 0 ; ix < nxsecs ;  ix++ )
  {
	vec3d le_pnt1 = surf.get_pnt(ix,half_ind).transform(model_mat);
	vec3d le_pnt2 = surf.get_pnt(ix+1,half_ind).transform(model_mat);

	vec3d te_pnt1 = surf.get_pnt(ix,0).transform(model_mat);
	vec3d te_pnt2 = surf.get_pnt(ix+1,0).transform(model_mat);

	double chord1 = dist( te_pnt1, le_pnt1 );
	double chord2 = dist( te_pnt2, le_pnt2 );

  	double ds_root_le = chord()/40.;
  	double ds1 = MAX(chord1/chord(),0.50)*ds_root_le;
  	double ds2 = MAX(chord2/chord(),0.50)*ds_root_le;

  	ds1 = MAX(ds1, inlet_dia()*3/1000.);
	ds2 = MAX(ds2, inlet_dia()*3/1000.);

  	//==== Write Out Line Along Leading Edge ====//
  	fprintf(dump_file, " %s: Leading Edge Line %d \n", (char*)getName(), ix);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt1.x(), le_pnt1.y(), le_pnt1.z(), ds1, 2.*ds1 ,9.*ds1);
  	fprintf(dump_file, "    %f    %f    %f    %f    %f    %f  \n",
      le_pnt2.x(), le_pnt2.y(), le_pnt2.z(), ds2, 2.*ds2 ,9.*ds2);

	}


}

int DuctGeom::get_num_felisa_line_sources()
{
  int nls = get_num_felisa_comps();
  return nls;
}

//==== Write Out Felisa Background Triangle File ====//
void DuctGeom::write_felisa_tri_sources(FILE* dump_file)
{

  int ix;
  int nxsecs = get_num_felisa_comps()/2;
  int half_ind = surf.get_num_pnts()/2;

  double span = inlet_dia()*2;
  span = MAX(0.5*span,chord());

  //====  Write Out Tris ====//
  for ( ix = 0 ; ix < nxsecs ;  ix++ )
  {
	vec3d te_pnt1 = surf.get_pnt(ix,0).transform(model_mat);
	vec3d te_pnt2 = surf.get_pnt(ix+1,0).transform(model_mat);

	vec3d le_pnt1 = surf.get_pnt(ix,half_ind).transform(model_mat);
	vec3d le_pnt2 = surf.get_pnt(ix+1,half_ind).transform(model_mat);

	double chord1 = dist( te_pnt1, le_pnt1 );
	double chord2 = dist( te_pnt2, le_pnt2 );

    double ds_root = chord()/30.;
  	double ds1 = ds_root;
  	double ds2 = ds_root;

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

int DuctGeom::get_num_felisa_tri_sources()
{
	int numsurf = get_num_felisa_comps();		
	return ( numsurf );						// NumTri = 2.0*(numsurf/2.0)
}


//==== Aero Ref Parameters ====//
double DuctGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		double avg_dia = (inlet_dia() + outlet_dia())*0.5;
		refArea = avg_dia*length();
	}
	return refArea;
}
	
double DuctGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		double avg_dia = (inlet_dia() + outlet_dia())*0.5;
		refSpan = avg_dia;
	}
	return refSpan;
}

double DuctGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		refCbar = chord();
	}
	return refCbar;
}

vec3d DuctGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		double len = length()*scaleFactor(); 
		aeroCenter.set_x( length()*0.25 + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void DuctGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	int np = surf.get_num_pnts();
	vec3d p0 = surf.get_pnt( 0, np/4 );
	vec3d p1 = surf.get_pnt( 0, 3*np/4 );
	vec3d p = (p0 + p1)*0.5;
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }

}


