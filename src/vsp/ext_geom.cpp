//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//    External Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/97
//   Sterling Software
//
//******************************************************************************

#include "ext_geom.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "parmLinkMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"

//==== Constructor =====//
Ext_geom::Ext_geom(Aircraft* aptr) : Geom(aptr)
{
  type = EXT_GEOM_TYPE;
	type_str = Stringc("ext");

  char name[255];
  sprintf( name, "Ext_%d", geomCnt ); 
  geomCnt++;
  name_str = Stringc(name);
  id_str = name_str;				//jrg fix
  setSymCode(XZ_SYM);
  yLoc = 2.0;

  setNumPntsXsecs();
  define_parms();

  generate();

  surfVec.push_back( &body_surf );
  surfVec.push_back( &pylon_surf );
  surfVec.push_back( &fin1_surf );
  surfVec.push_back( &fin2_surf );



}

//==== Destructor =====//
Ext_geom::~Ext_geom()
{
   delete pylon_af; 
   delete fin_af;   
}

void Ext_geom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != EXT_GEOM_TYPE )
		return;

	Ext_geom* eg = (Ext_geom*)fromGeom;

    ext_type = eg->get_ext_type();
	pylon_flag = eg->get_pylon_flag();

	length.set( eg->length() );
	fine_ratio.set( eg->fine_ratio() );
	pylon_height.set( eg->pylon_height() );
	drag.set( eg->drag() );
	pylon_drag.set( eg->pylon_drag() );

	body_surf.set_num_pnts( numPnts.iget() );
	body_surf.set_num_xsecs(  numXsecs.iget() );
	setNumPntsXsecs();

	compose_model_matrix();
	generate();

}

void Ext_geom::setNumPntsXsecs()
{
  numPntsBody   = numPnts.iget();
  numXsecsBody  = numXsecs.iget();
  numPntsPylon  = numPnts.iget()/2 + 1;
  numXsecsPylon = 4;
  numPntsFin    = numPnts.iget()/2 + 1;
  numXsecsFin   = 5;
}

//==== Define  Parms ====//
void Ext_geom::define_parms()
{
  circle.set_num_pnts(numPntsBody);
  circle.set_type(CIRCLE);
  circle.get_height()->set(1.0); 
  circle.get_width()->set(1.0);
  circle.generate();

  body_curve.init_num_sections(2);

  ext_type = MISSLE_TYPE;
  pylon_flag = 1;
 
  compose_model_matrix();

  length.initialize(this, "Length", 6.5);
  length.set_lower_upper(0.0001, 1000000.0);
  length.set_script("ext_design length", 0);


  fine_ratio.initialize(this, "Finess_Ratio", 15.0);
  fine_ratio.set_lower_upper(2.0, 50.0);
  fine_ratio.set_script("ext_design finess", 0);

  pylon_height.initialize(this, "Pylon_Height", 0.35);
  pylon_height.set_lower_upper(0.001, 200000.0);
  pylon_height.set_script("ext_design pylonheight", 0);

  drag.initialize(this, "Flat_Plate_Drag", 0.01);
  drag.set_script("ext_design cdplate", 0);
  pylon_drag.initialize(this, "Pylon_Drag", 0.01);
  pylon_drag.set_script("ext_design cdplate", 0);
  drag.set_lower_upper(0.0, 100.0);
  pylon_drag.set_lower_upper(0.0, 100.0);

  body_surf.set_num_pnts( numPntsBody );
  body_surf.set_num_xsecs(  numXsecsBody );

  pylon_af= new Af(this);
  pylon_af->set_type( NACA_4_SERIES );
  pylon_af->set_num_pnts( numPntsPylon );
  pylon_af->get_camber()->set( 0.0 );
  pylon_af->get_thickness()->set( 0.10 );

  fin_af= new Af(this);
  fin_af->set_type( NACA_4_SERIES );
  fin_af->set_num_pnts( numPntsFin );
  fin_af->get_camber()->set( 0.0 );
  fin_af->get_thickness()->set( 0.05 );

}

void Ext_geom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );
	Geom::AddLinkableParm( &fine_ratio, parmVec, this, "Design" );
	Geom::AddLinkableParm( &pylon_height, parmVec, this, "Design" );
}

//==== Generate Fuse Component ====//
void Ext_geom::generate()
{
  generate_body_curve();
  generate_pylon();
  generate_fins();
  update_bbox();
}

void Ext_geom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*length()*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void Ext_geom::parm_changed(Parm* p)
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
		numPnts = 4*((int)numPnts()/4)+1;
		setNumPntsXsecs();
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

void Ext_geom::set_ext_type(int type_in)
{
	ext_type = type_in; 
	generate();
	airPtr->geomMod( this );
}

void Ext_geom::set_pylon_flag(int flag_in)
{
	pylon_flag = flag_in; 
	generate();
	airPtr->geomMod( this );
}

//==== Generate Body Defining Curve ====//
void Ext_geom::generate_body_curve()
{
  double len = length(); 
  double dia = len/fine_ratio();

  //==== Load Points For Curves ====//
  body_curve.put_pnt(0, vec3d(0.0, 0.0, 0.0) );
  body_curve.put_pnt(3, vec3d(len/2.0, 0.0, dia) );
  body_curve.put_pnt(6, vec3d(len, 0.0, 0.0) );

  //==== Set Curve Def Parms  ====//
  if (ext_type == MISSLE_TYPE )
    {
      body_curve.put_pnt(1, vec3d(0.01*len, 0.0, dia*0.8) );
      body_curve.put_pnt(2, vec3d(0.05*len, 0.0, dia) );
      body_curve.put_pnt(4, vec3d(0.99*len, 0.0, dia) );
      body_curve.put_pnt(5, vec3d(1.00*len, 0.0, dia) );
    }
  else
    {
      body_curve.put_pnt(1, vec3d(0.05*len, 0.0, dia*0.5) );
      body_curve.put_pnt(2, vec3d(0.20*len, 0.0, dia) );
      body_curve.put_pnt(4, vec3d(0.80*len, 0.0, dia) );
      body_curve.put_pnt(5, vec3d(0.95*len, 0.0, dia*0.5) );
    }

  gen_body_surf();
 
}

//==== Generate Fin Surfaces ====//
void Ext_geom::generate_fins()
{
  if ( ext_type == TANK_TYPE || ext_type == FIXED_TANK_TYPE )
    {
      fin1_surf.set_num_pnts( 0);
      fin1_surf.set_num_xsecs(0);
      fin2_surf.set_num_pnts( 0);
      fin2_surf.set_num_xsecs(0);
      return; 
    } 
  fin1_surf.set_num_pnts(  numPntsFin );
  fin1_surf.set_num_xsecs( numXsecsFin );
  fin2_surf.set_num_pnts(  numPntsFin );
  fin2_surf.set_num_xsecs( numXsecsFin );
  fin_af->set_num_pnts( numPntsFin );

  vec3d tmp_pnt;
  double rt_chd = 0.20*length();
  double tip_chd = 0.15*length();
 
  double rt_off = 0.75*length();
  double tip_off = 0.85*length();

  double dia = length()/fine_ratio();
  double span = dia*0.8; \
  fin_af->get_thickness()->set(0.5/fine_ratio());
  fin_af->generate_airfoil();
  int i;

  for (  i = 0 ; i < numPntsFin ; i++ )
    {
      tmp_pnt = fin_af->get_end_cap(i);
      fin1_surf.set_pnt(4, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, span, tmp_pnt.z()*tip_chd) );  
      fin2_surf.set_pnt(4, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, span, tmp_pnt.z()*tip_chd) );  
      fin1_surf.set_pnt(0, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, -span, tmp_pnt.z()*tip_chd) );  
      fin2_surf.set_pnt(0, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, -span, tmp_pnt.z()*tip_chd) );  
      tmp_pnt = fin_af->get_pnt(i);
      fin1_surf.set_pnt(3, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, span, tmp_pnt.z()*tip_chd) );  
      fin2_surf.set_pnt(3, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, span, tmp_pnt.z()*tip_chd) );  
      fin1_surf.set_pnt(2, i, vec3d(tmp_pnt.x()*rt_chd+rt_off, 0, tmp_pnt.z()*rt_chd) );  
      fin2_surf.set_pnt(2, i, vec3d(tmp_pnt.x()*rt_chd+rt_off, 0, tmp_pnt.z()*rt_chd) );  
      fin1_surf.set_pnt(1, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, -span, tmp_pnt.z()*tip_chd) );  
      fin2_surf.set_pnt(1, i, vec3d(tmp_pnt.x()*tip_chd+tip_off, -span, tmp_pnt.z()*tip_chd) );  
    }    
 
  for ( i = 0 ; i < numXsecsFin ; i++ )
    {
      fin1_surf.rotate_xsec_x(i,  45.0);
      fin2_surf.rotate_xsec_x(i, -45.0);
    }

  fin1_surf.load_refl_pnts_xsecs();
  fin2_surf.load_refl_pnts_xsecs();
//jrg  if (main_window_ptr->get_display_type_flag() == DISPLAY_HIDDEN_LINE)
    {
      fin1_surf.load_hidden_surf();
      fin2_surf.load_hidden_surf();
    }
//jrg  else if (main_window_ptr->get_display_type_flag() == DISPLAY_SHADED_SURF)
    {
      fin1_surf.load_normals();
      fin2_surf.load_normals();
    }

	fin1_surf.load_uw();
	fin2_surf.load_uw();

}

//==== Generate Pylon Surface ====//
void Ext_geom::generate_pylon()
{
  if ( !pylon_flag )
    {
      pylon_surf.set_num_pnts(  0 );
      pylon_surf.set_num_xsecs( 0 );
      return; 
    } 
  pylon_surf.set_num_pnts(  numPntsPylon );
  pylon_surf.set_num_xsecs( numXsecsPylon );
  pylon_af->set_num_pnts( numPntsPylon );

  vec3d tmp_pnt;
  double chord = 0.3*length();
  double dia = length()/fine_ratio();
  double offset = 0.35*length();
  double start_z = 0.40*dia;
  double end_z   = 0.50*dia + pylon_height();
  pylon_af->get_thickness()->set(1.0/fine_ratio());
  pylon_af->generate_airfoil();


  for ( int i = 0 ; i < numPntsPylon ; i++ )
    {
      tmp_pnt = pylon_af->get_pnt(i);
      pylon_surf.set_pnt(2, i, vec3d(tmp_pnt.x()*chord+offset, tmp_pnt.z()*chord, start_z) );  
      pylon_surf.set_pnt(1, i, vec3d(tmp_pnt.x()*chord+offset, tmp_pnt.z()*chord, end_z) ); 
	  
	  tmp_pnt = pylon_af->get_end_cap( i );
      pylon_surf.set_pnt(3, i, vec3d(tmp_pnt.x()*chord+offset, tmp_pnt.z()*chord, start_z) );  
      pylon_surf.set_pnt(0, i, vec3d(tmp_pnt.x()*chord+offset, tmp_pnt.z()*chord, end_z) );  
    }    
 
  pylon_surf.load_refl_pnts_xsecs();
//jrg  if (main_window_ptr->get_display_type_flag() == DISPLAY_HIDDEN_LINE)
    pylon_surf.load_hidden_surf();
//jrg  else if (main_window_ptr->get_display_type_flag() == DISPLAY_SHADED_SURF)
    pylon_surf.load_normals();

	pylon_surf.load_uw();


}

//==== Write Fuse File ====//
void Ext_geom::write(FILE* file_id)
{
/*
  fprintf(file_id, "//=================== EXTERNAL COMPONENT ====================//\n");

  //==== Write General Parms ====// 
  write_general_parms(file_id);

  fprintf(file_id, "%d				External_Type\n",ext_type);
  fprintf(file_id, "%d				Pylon_Flag\n",pylon_flag);
  fprintf(file_id, "%f			Length (ft)\n",(float)((Feet)length()));
  fprintf(file_id, "%f			Finess_Ratio\n",fine_ratio());
  fprintf(file_id, "%f			Pylon_Height (ft)\n",(float)((Feet)pylon_height()));
  fprintf(file_id, "%f			Drag\n",drag());
  fprintf(file_id, "%f			Pylon_Drag\n",pylon_drag());
*/  
}

//==== Write External File ====//
void Ext_geom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "External");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr ext_node = xmlNewChild( root, NULL, (const xmlChar *)"External_Parms", NULL );

  xmlAddIntNode( ext_node, "External_Type", ext_type );
  xmlAddIntNode( ext_node, "Pylon_Flag", pylon_flag );
  xmlAddDoubleNode( ext_node, "Length", length() );
  xmlAddDoubleNode( ext_node, "Finess_Ratio", fine_ratio() );
  xmlAddDoubleNode( ext_node, "Pylon_Height", pylon_height() );
  xmlAddDoubleNode( ext_node, "Drag", drag() );
  xmlAddDoubleNode( ext_node, "Pylon_Drag", pylon_drag() );

}
//==== Read External File ====//
void Ext_geom::read(xmlNodePtr root)
{

  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "External_Parms", 0 );
  if ( node )
  {
    ext_type   =  xmlFindInt( node, "External_Type", ext_type );
    pylon_flag =  xmlFindInt( node, "Pylon_Flag", pylon_flag );

    length = xmlFindDouble( node, "Length", length() );
    fine_ratio = xmlFindDouble( node, "Finess_Ratio", fine_ratio() );
    pylon_height = xmlFindDouble( node, "Pylon_Height", pylon_height() );
    drag = xmlFindDouble( node, "Drag", drag() );
    pylon_drag = xmlFindDouble( node, "Pylon_Drag", pylon_drag() );
  }
  generate();

}

//==== Read Fuse File ====//
void Ext_geom::read(FILE* file_id)
{
  char buff[255];

  //==== Read General Parms ====// 
  read_general_parms(file_id);

  //==== Read External Parms ====//		
  fscanf(file_id, "%d",&ext_type); 			fgets(buff, 80, file_id);

  fscanf(file_id, "%d",&pylon_flag); 			fgets(buff, 80, file_id);

  length.read(file_id);
  fine_ratio.read(file_id);
  pylon_height.read(file_id);
  drag.read(file_id);
  pylon_drag.read(file_id);

  generate();

}

//==== Write Rhino File ====//
void Ext_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	int j;

	//==== Body ====//
	body_surf.clear_xsec_tan_flags();
	body_surf.set_xsec_tan_flag( 0, Bezier_curve::ZERO );
    int num_xsecs  = body_surf.get_num_xsecs();
	body_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );
	body_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );

	//==== Fin 1  ====//
	fin1_surf.clear_pnt_tan_flags();
	fin1_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	fin1_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    int num_pnts  = fin1_surf.get_num_pnts();
	fin1_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	//==== Sharpen Wing Joints ====//
	num_xsecs = fin1_surf.get_num_xsecs();
	for ( j = 0 ; j < num_xsecs ; j++ )
		fin1_surf.set_xsec_tan_flag( j, Bezier_curve::SHARP );

	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3 );
	u_split.push_back( 3*(fin1_surf.get_num_xsecs() - 2) );
	u_split.push_back( 3*(fin1_surf.get_num_xsecs() - 1) );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(fin1_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(fin1_surf.get_num_pnts() - 1)  );

	//==== Write File ====//
	if ( ext_type == MISSLE_TYPE || ext_type == BOMB_TYPE )
	{
		fin1_surf.write_split_rhino_file( sym_code, model_mat, reflect_mat, 
			u_split, w_split, false, archive, attributes );
	}

	//==== Fin 2  ====//
	fin2_surf.clear_pnt_tan_flags();
	fin2_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	fin2_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    num_pnts  = fin2_surf.get_num_pnts();
	fin2_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	//==== Sharpen Wing Joints ====//
	num_xsecs = fin2_surf.get_num_xsecs();

	for ( j = 0 ; j < num_xsecs ; j++ )
		fin2_surf.set_xsec_tan_flag( j, Bezier_curve::SHARP );

	u_split.clear();
	u_split.push_back( 0 );
	u_split.push_back( 3 );
	u_split.push_back( 3*(fin2_surf.get_num_xsecs() - 2) );
	u_split.push_back( 3*(fin2_surf.get_num_xsecs() - 1) );
	w_split.clear();
	w_split.push_back( 0 );
	w_split.push_back( 3*(fin2_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(fin2_surf.get_num_pnts() - 1)  );

	//==== Write File ====//
	if ( ext_type == MISSLE_TYPE || ext_type == BOMB_TYPE )
	{
		fin2_surf.write_split_rhino_file( sym_code, model_mat, reflect_mat, 
			u_split, w_split, false, archive, attributes );
	}


	//==== Pylon ====//
	pylon_surf.clear_pnt_tan_flags();
	pylon_surf.clear_xsec_tan_flags();

	//==== Sharpen Trailing Edge of Wing ====//
	pylon_surf.set_pnt_tan_flag( 0, Bezier_curve::SHARP );
    num_pnts  = pylon_surf.get_num_pnts();
	pylon_surf.set_pnt_tan_flag( num_pnts-1, Bezier_curve::SHARP );

	//==== Sharpen Wing Joints ====//
	num_xsecs = pylon_surf.get_num_xsecs();

	for ( j = 0 ; j < num_xsecs ; j++ )
		pylon_surf.set_xsec_tan_flag( j, Bezier_curve::SHARP );

	u_split.clear();
	u_split.push_back( 0 );
	u_split.push_back( 3 );
	u_split.push_back( 3*(pylon_surf.get_num_xsecs() - 2) );
	u_split.push_back( 3*(pylon_surf.get_num_xsecs() - 1) );
	w_split.clear();
	w_split.push_back( 0 );
	w_split.push_back( 3*(pylon_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*(pylon_surf.get_num_pnts() - 1)  );

	if ( pylon_flag )
	{
		pylon_surf.write_split_rhino_file( sym_code, model_mat, reflect_mat, 
			u_split, w_split, false, archive, attributes );
	}
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > Ext_geom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	tMeshVec.push_back( body_surf.createTMesh(model_mat) );

	if ( ext_type == MISSLE_TYPE || ext_type == BOMB_TYPE )
	{	
		tMeshVec.push_back( fin1_surf.createTMesh(model_mat) );
		tMeshVec.push_back( fin2_surf.createTMesh(model_mat) );
	}
	if ( pylon_flag )
		tMeshVec.push_back( pylon_surf.createTMesh(model_mat) );

	if ( sym_code != NO_SYM )
	{
		tMeshVec.push_back( body_surf.createReflTMesh(sym_code, reflect_mat) );
		if ( ext_type == MISSLE_TYPE || ext_type == BOMB_TYPE )
		{	
			tMeshVec.push_back( fin1_surf.createReflTMesh(sym_code, reflect_mat) );
			tMeshVec.push_back( fin2_surf.createReflTMesh(sym_code, reflect_mat) );
		}
		if ( pylon_flag )
			tMeshVec.push_back( pylon_surf.createReflTMesh(sym_code, reflect_mat) );
	}
	return tMeshVec;
}

//==== Generate Cross Sections =====//
void Ext_geom::regenerate()
{
}

//==== Generate Cross Sections =====//
void Ext_geom::gen_body_surf()
{
  float u;
  vec3d def_pnt, temp_pnt;

  circle.set_num_pnts(numPntsBody);
  body_surf.set_num_pnts(  numPntsBody );
  body_surf.set_num_xsecs( numXsecsBody );

  //==== Set Cross Section Parms And Loads X-Secs ====//
  int nx_1 = numXsecsBody/2 + 1;
  int nx_2 = numXsecsBody - nx_1;
  int i;

  for (  i = 0 ; i < nx_1 ; i++ )
    {
      u = (float)i/(float)(nx_1 - 1);
      def_pnt = body_curve.comp_pnt(0, u);
      for ( int j = 0 ; j < numPntsBody ; j++ )
        {
          temp_pnt = circle.get_pnt(j)*def_pnt.z();
          temp_pnt.set_x(def_pnt.x());      
          body_surf.set_pnt(i, j, temp_pnt);
        }
    }

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for ( i = 0 ; i < nx_2 ; i++ )
    {
      u = (float)(i+1)/(float)(nx_2);
      def_pnt = body_curve.comp_pnt(1, u);
       for ( int j = 0 ; j < numPntsBody ; j++ )
        {
          temp_pnt = circle.get_pnt(j)*def_pnt.z();
          temp_pnt.set_x(def_pnt.x());      
          body_surf.set_pnt(i+nx_1, j, temp_pnt);
        }
    }

  body_surf.load_refl_pnts_xsecs();
//  if (main_window_ptr->get_display_type_flag() == DISPLAY_HIDDEN_LINE)
    body_surf.load_hidden_surf();
//  else if (main_window_ptr->get_display_type_flag() == DISPLAY_SHADED_SURF)
    body_surf.load_normals();

    body_surf.load_uw();


}


//==== Generate Cross Sections =====//
void Ext_geom::update_bbox()
{
  vec3d tmp_pnt;
  bbox new_box;

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < numPntsBody ; j++ )
    {
      tmp_pnt = body_surf.get_pnt(0,j);               new_box.update(tmp_pnt);
      tmp_pnt = body_surf.get_pnt(numXsecsBody/2,j); new_box.update(tmp_pnt);
      tmp_pnt = body_surf.get_pnt(numXsecsBody-1,j);   new_box.update(tmp_pnt);
     }

  bnd_box = new_box;
}


void Ext_geom::draw()
{
	Geom::draw();
/*
	if ( fastDrawFlag )
	{
		body_surf.fast_draw_on();
		pylon_surf.fast_draw_on();
		fin1_surf.fast_draw_on();
		fin2_surf.fast_draw_on();
	}
	else
	{
		body_surf.fast_draw_off();
		pylon_surf.fast_draw_off();
		fin1_surf.fast_draw_off();
		fin2_surf.fast_draw_off();
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
		body_surf.draw_wire();
		pylon_surf.draw_wire();
		fin1_surf.draw_wire();
		fin2_surf.draw_wire();
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		body_surf.draw_refl_wire(sym_code);
		pylon_surf.draw_refl_wire(sym_code);
		fin1_surf.draw_refl_wire(sym_code);
		fin2_surf.draw_refl_wire(sym_code);
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
				body_surf.draw_shaded();
				pylon_surf.draw_shaded();
				fin1_surf.draw_shaded();
				fin2_surf.draw_shaded();
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
				body_surf.draw_refl_shaded( sym_code);
				pylon_surf.draw_refl_shaded( sym_code);
				fin1_surf.draw_refl_shaded( sym_code);
				fin2_surf.draw_refl_shaded( sym_code);
			}
		}
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_HIDDEN_FLAG )
	{
		//==== Draw Hidden Surface ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		body_surf.draw_hidden();
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		pylon_surf.draw_hidden();
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		fin1_surf.draw_hidden();
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		fin2_surf.draw_hidden();
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		body_surf.draw_refl_hidden(sym_code);
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		pylon_surf.draw_refl_hidden(sym_code);
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		fin1_surf.draw_refl_hidden(sym_code);
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		fin2_surf.draw_refl_hidden(sym_code);
		glPopMatrix();
			
	}
*/
}

//==== Draw If Alpha < 1 and Shaded ====//
void Ext_geom::drawAlpha()
{
	Geom::drawAlpha();
/*
	if ( displayFlag != GEOM_SHADE_FLAG )
		return;

	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( mat && mat->diff[3] <= 0.99 )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();
		body_surf.draw_shaded();
		pylon_surf.draw_shaded();
		fin1_surf.draw_shaded();
		fin2_surf.draw_shaded();

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		body_surf.draw_refl_shaded( sym_code);
		pylon_surf.draw_refl_shaded( sym_code);
		fin1_surf.draw_refl_shaded( sym_code);
		fin2_surf.draw_refl_shaded( sym_code);

		glPopMatrix();
	}
*/
}

//==== Compute And Load Normals ====//
void Ext_geom::load_normals()
{
  body_surf.load_normals();
  pylon_surf.load_normals();
  fin1_surf.load_normals();
  fin2_surf.load_normals();
}

//==== Draw Other Pnts XSecs====//
void Ext_geom::load_hidden_surf()
{
  body_surf.load_hidden_surf();
  pylon_surf.load_hidden_surf();
  fin1_surf.load_hidden_surf();
  fin2_surf.load_hidden_surf();
}

//==== Get Area ====//
float Ext_geom::get_area()
{
  return (float)( length()*length()/fine_ratio() );
}

//==== Get Span ====//
float Ext_geom::get_span()
{
  return (float)( length()/fine_ratio() );
}

//==== Get Cbar ====//
float  Ext_geom::get_cbar()
{
  return (float)( length() );
}

//==== Write Aero File =====//
void Ext_geom::write_aero_file(FILE* aero_file, int aero_id_num)
{
/* jrg 
  fprintf(aero_file, "\n******************************************************************************************\n");
  fprintf(aero_file, "*GENERAL PARAMETER*    *DESCRIPTION*\n");
  fprintf(aero_file, "%d			Component Number\n",aero_id_num);
  int name_length = name.get_length();
  Stringc temp_name = name;
  while ( name_length < 15 )
  {
    temp_name.concatenate(" ");
    name_length++;
  }
  fprintf(aero_file, "2			Componet Type\n");
  fprintf(aero_file, "%s		Componet Name\n", (char*) temp_name);
  fprintf(aero_file, "%s		Componet Identifier\n",(char*) id_str);

  int reflect = 0;
  if ( sym_code == XZ_SYM && (Feet)get_tran_y() == 0.0 )
  {
    reflect = 1;
  }
  else if ( sym_code == XZ_SYM && (Feet)get_tran_y() != 0.0 )
  {
    reflect = 2;
  }
  fprintf(aero_file, "%d			XZ Plane Reflection Flag\n",reflect);
  fprintf(aero_file, "%d			Reference Compenent Flag\n",0);
  fprintf(aero_file, "%f		X Location of Nose\n",(float)((Feet)get_tran_x()) );
  fprintf(aero_file, "%f		Y Location of Nose\n",(float)((Feet)get_tran_y()) );
  fprintf(aero_file, "%f		Z Location of Nose\n",(float)((Feet)get_tran_z()) );
  float xrot = (Degrees)get_rot_x();
  while (xrot > 360.0)  xrot -= 360.0;
  while (xrot < 0.0)  xrot += 360.0;
  fprintf(aero_file, "%f		X Rotation (Deg)\n",xrot);
  fprintf(aero_file, "%f		Y Rotation (Deg)\n",(float)((Degrees)get_rot_y()) );
  fprintf(aero_file, "%f		Z Rotation (Deg)\n",(float)((Degrees)get_rot_z()) );
  fprintf(aero_file, "==========================================================================================\n");
  fprintf(aero_file, "*EXTERNAL PARAMETER*   *DESCRIPTION*\n");
//  fprintf(aero_file, "%d			EXTYPE		External Type\n",get_ext_type()+1);
  fprintf(aero_file, "%f		Flat Plate Drag (D/q)\n",get_drag()->get());
  fprintf(aero_file, "%f		Pylon Flat Plate Drag (D/q)\n",get_pylon_drag()->get());
*/
}


void Ext_geom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	length.set( length()*current_factor );
    generate();

	lastScaleFactor = scaleFactor();
}

void Ext_geom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void Ext_geom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

vec3d Ext_geom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = body_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}
/*
vec3d Ext_geom::getAttachFixedPos()
{
	vec3d pos;
	pos.set_xyz( xLoc() + xLocOff(), 
				 yLoc() + yLocOff(), 
				 zLoc() + zLocOff());
	return pos;
}
*/

vec3d Ext_geom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	switch (surfid)
	{
		case BODY_SURF:
			return body_surf.get_vertex(x, p, r);
		case PYLON_SURF:
			return pylon_surf.get_vertex(x, p, r);
		case FIN1_SURF:
			return fin1_surf.get_vertex(x, p, r);
		case FIN2_SURF:
			return fin2_surf.get_vertex(x, p, r);
		default:
			return vec3d(0,0,0);
	}
}

void  Ext_geom::getVertexVec(vector< VertexID > *vertVec)
{ 
	buildVertexVec(&body_surf, BODY_SURF, vertVec); 
	buildVertexVec(&pylon_surf, PYLON_SURF, vertVec); 
	buildVertexVec(&fin1_surf, FIN1_SURF, vertVec); 
	buildVertexVec(&fin2_surf, FIN2_SURF, vertVec); 
}
