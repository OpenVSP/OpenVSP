//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Airfoil Class II
//  
// 
//   J.R. Gloudemans - 8/2/93
//   Sterling Software
//
//******************************************************************************

#include <math.h>

#include "af.h"
#include "defines.h"
#include "aircraft.h"

//===== Constructor  =====//
Af::Af(Geom* geom_ptr_in)
{
  geom_ptr = geom_ptr_in;

  //==== Defaults ====//
  type = NACA_4_SERIES;
  num_pnts = 23;

  curve_loaded_flag = FALSE;
  inverted_flag = FALSE;

  camber.initialize(geom_ptr, AF_UPDATE_GROUP, "Camber",  0.0);
  camber.set_lower_upper(0.0, 0.5);

  camber_loc.initialize(geom_ptr, AF_UPDATE_GROUP, "Camber_Loc",  0.5);
  camber_loc.set_lower_upper(0.01, 0.99);

  thickness.initialize(geom_ptr, AF_UPDATE_GROUP, "Thickness",  0.10);
  thickness.set_lower_upper(0.001, 0.5);
  thickness_loc.deactivate();

  thickness_loc.initialize(geom_ptr, AF_UPDATE_GROUP, "Thickness_Loc",  0.3);
  thickness_loc.set_lower_upper(0.01, 0.99);

  radius_le.initialize(geom_ptr, AF_UPDATE_GROUP, "Radius_Leading_Edge",  0.01);
  radius_le.set_lower_upper(0.0, 1000000.0);
  radius_le.deactivate();

  radius_te.initialize(geom_ptr, AF_UPDATE_GROUP, "Radius_Trailing_Edge",  0.0);
  radius_te.set_lower_upper(0.0, 1000000.0);
  radius_te.deactivate();

  delta_y_le.initialize(geom_ptr, AF_UPDATE_GROUP, "Delta_Y_Leading_Edge",  0.01);
  delta_y_le.set_lower_upper(0.0, 1000000.0);

  ideal_cl.initialize(geom_ptr, AF_UPDATE_GROUP, "Ideal Cl",  0.0);
  ideal_cl.set_lower_upper(0.0, 1.0);
  ideal_cl.deactivate();

  a.initialize(geom_ptr, AF_UPDATE_GROUP, "A",  0.0);
  a.set_lower_upper(0.0, 1.0);
  a.deactivate();

  sixser = 63;

  slat_flag = FALSE;
  slat_shear_flag = FALSE;
  slat_chord.initialize( geom_ptr, AF_UPDATE_GROUP, "Slat_Chord",  0.25 );
  slat_chord.set_lower_upper(0.0, 1.0);
  slat_angle.initialize( geom_ptr, AF_UPDATE_GROUP, "Slat_Angle",  10.0 );
  slat_angle.set_lower_upper(-45.0, 45.0);

  flap_flag = FALSE;
  flap_shear_flag = FALSE;
  flap_chord.initialize( geom_ptr, AF_UPDATE_GROUP, "Flap_Chord",  0.25 );
  flap_chord.set_lower_upper(0.0, 1.0);
  flap_angle.initialize( geom_ptr, AF_UPDATE_GROUP, "Flap_Angle",  10.0 );
  flap_angle.set_lower_upper(-45.0, 45.0);

  load_name(); 
  generate_airfoil();

}

//===== Destructor  =====//
Af::~Af()
{
}

//===== Copy  =====//
Af& Af::operator=(const Af& iaf) //jrg - look at this...
{
  type = iaf.type;
  curve_loaded_flag = iaf.curve_loaded_flag;
  inverted_flag = iaf.inverted_flag;

  camber = iaf.camber;
  camber_loc = iaf.camber_loc;
  thickness = iaf.thickness;
  thickness_loc = iaf.thickness_loc;
  radius_le = iaf.radius_le;
  radius_te = iaf.radius_te;
  delta_y_le = iaf.delta_y_le;
  a = iaf.a;
  ideal_cl = iaf.ideal_cl;

  sixser = iaf.sixser;
  name = iaf.name;

  num_pnts = iaf.num_pnts;

  orig_af_thickness = iaf.orig_af_thickness;
  radius_le_correction_factor = iaf.radius_le_correction_factor;
  radius_te_correction_factor = iaf.radius_te_correction_factor;

  upper_curve = iaf.upper_curve;
  lower_curve = iaf.lower_curve;

  slat_flag = iaf.slat_flag;
  slat_shear_flag = iaf.slat_shear_flag;
  slat_chord = iaf.slat_chord;
  slat_angle = iaf.slat_angle;

  flap_flag = iaf.flap_flag;
  flap_shear_flag = iaf.flap_shear_flag;
  flap_chord = iaf.flap_chord;
  flap_angle = iaf.flap_angle;

	set_geom(geom_ptr );

	load_name(); 
	generate_airfoil();

	return *this; 
}

vector< Parm* > Af::GetLinkableParms()
{
	vector< Parm* > pVec;
	pVec.push_back( get_camber() );
	pVec.push_back( get_camber_loc() );
	pVec.push_back( get_thickness() );
	pVec.push_back( get_thickness_loc() );
	pVec.push_back( get_ideal_cl() );
	pVec.push_back( get_a() );
	pVec.push_back( get_leading_edge_radius() );
	pVec.push_back( get_trailing_edge_radius() );
	pVec.push_back( get_slat_chord() );
	pVec.push_back( get_slat_angle() );
	pVec.push_back( get_flap_chord() );
	pVec.push_back( get_flap_angle() );

	return pVec;
}

void Af::set_geom(Geom * gPtr )
{
	geom_ptr = gPtr;
	camber.set_geom( geom_ptr );
	camber_loc.set_geom( geom_ptr );
	thickness.set_geom( geom_ptr );
	thickness_loc.set_geom( geom_ptr );
	radius_le.set_geom( geom_ptr );
	radius_te.set_geom( geom_ptr );
	delta_y_le.set_geom( geom_ptr );
	ideal_cl.set_geom( geom_ptr );
	a.set_geom( geom_ptr );
	slat_chord.set_geom( geom_ptr );
	slat_angle.set_geom( geom_ptr );
	flap_chord.set_geom( geom_ptr );
	flap_angle.set_geom( geom_ptr );
}

void Af::write(xmlNodePtr node)
{
  int i;

  xmlAddIntNode( node, "Type", type);
  xmlAddIntNode( node, "Inverted_Flag", inverted_flag);

  xmlAddDoubleNode( node, "Camber", camber() );
  xmlAddDoubleNode( node, "Camber_Loc", camber_loc() );
  xmlAddDoubleNode( node, "Thickness",  thickness() );
  xmlAddDoubleNode( node, "Thickness_Loc",  thickness_loc() );

  xmlAddDoubleNode( node, "Radius_Le",  radius_le() );
  xmlAddDoubleNode( node, "Radius_Te",  radius_te() );

  xmlAddIntNode( node, "Six_Series",  sixser );
  xmlAddDoubleNode( node, "Ideal_Cl",  ideal_cl() );
  xmlAddDoubleNode( node, "A",  a() );

  if ( type == AIRFOIL_FILE )
  {
    xmlAddStringNode( node, "Name",  name() );
    xmlAddDoubleNode( node, "Original_AF_Thickness",  orig_af_thickness );
    xmlAddDoubleNode( node, "Radius_LE_Correction_Factor",  radius_le_correction_factor );
    xmlAddDoubleNode( node, "Radius_TE_Correction_Factor",  radius_te_correction_factor );

    vec3d pnt;
    Stringc upstr;
    char numstr[255];
    for (  i = 0 ; i < upper_curve.get_num_pnts() ; i++)
    {
      pnt = upper_curve.get_pnt(i);
      sprintf( numstr, "%lf, %lf,", pnt.x(), pnt.z() );
      upstr.concatenate(numstr);
    }
    upstr.concatenate("\0");

    xmlAddStringNode( node, "Upper_Pnts", upstr );

   Stringc lowstr;
    for (  i = 0 ; i < lower_curve.get_num_pnts() ; i++)
    {
      pnt = lower_curve.get_pnt(i);
      sprintf( numstr, "%lf, %lf,", pnt.x(), pnt.z() );
      lowstr.concatenate(numstr);
    }
    lowstr.concatenate("\0");

    xmlAddStringNode( node, "Lower_Pnts", lowstr );
  }

  xmlAddIntNode( node, "Slat_Flag",  slat_flag );
  xmlAddIntNode( node, "Slat_Shear_Flag",  slat_shear_flag );
  xmlAddDoubleNode( node, "Slat_Chord",  slat_chord() );
  xmlAddDoubleNode( node, "Slat_Angle",  slat_angle() );

  xmlAddIntNode( node, "Flap_Flag",  flap_flag );
  xmlAddIntNode( node, "Flap_Shear_Flag",  flap_shear_flag );
  xmlAddDoubleNode( node, "Flap_Chord",  flap_chord() );
  xmlAddDoubleNode( node, "Flap_Angle",  flap_angle() );

}

void Af::read(xmlNodePtr node)
{
  int i;

  int temp_type = xmlFindInt( node, "Type", 0 );
  inverted_flag = xmlFindInt( node, "Inverted_Flag", inverted_flag );

  camber        = xmlFindDouble( node, "Camber", camber() );
  camber_loc    = xmlFindDouble( node, "Camber_Loc", camber_loc() );
  thickness     = xmlFindDouble( node, "Thickness", thickness() );
  thickness_loc = xmlFindDouble( node, "Thickness_Loc", thickness_loc() );

  radius_le    = xmlFindDouble( node, "Radius_Le", radius_le() );
  radius_te    = xmlFindDouble( node, "Radius_Te", radius_te() );

  sixser = xmlFindInt( node, "Six_Series", sixser );
  ideal_cl = xmlFindDouble( node, "Ideal_Cl", ideal_cl() );
  a = xmlFindDouble( node, "A", a() );

  slat_flag = xmlFindInt( node, "Slat_Flag", slat_flag );
  slat_shear_flag = xmlFindInt( node, "Slat_Shear_Flag", slat_shear_flag );
  slat_chord = xmlFindDouble( node, "Slat_Chord", slat_chord() );
  slat_angle = xmlFindDouble( node, "Slat_Angle", slat_angle() );

  flap_flag = xmlFindInt( node, "Flap_Flag", flap_flag );
  flap_shear_flag = xmlFindInt( node, "Flap_Shear_Flag", flap_shear_flag );
  flap_chord = xmlFindDouble( node, "Flap_Chord", flap_chord() );
  flap_angle = xmlFindDouble( node, "Flap_Angle", flap_angle() );

  if ( temp_type == AIRFOIL_FILE )
  {
    vec3d pnt;
    name = xmlFindString( node, "Name", name );
    orig_af_thickness = (float)xmlFindDouble( node, "Original_AF_Thickness", orig_af_thickness );
    radius_le_correction_factor = (float)xmlFindDouble( node, "Radius_LE_Correction_Factor", radius_le_correction_factor );
    radius_te_correction_factor = (float)xmlFindDouble( node, "Radius_TE_Correction_Factor", radius_te_correction_factor );

    xmlNodePtr upp_node = xmlGetNode( node, "Upper_Pnts", 0 );

    if ( upp_node )
    {
      int num_arr =  xmlGetNumArray( upp_node, ',' );
      double* arr = (double*)malloc( num_arr*sizeof(double) );
      xmlExtractDoubleArray( upp_node, ',', arr, num_arr );

      int num_upper = num_arr/2;

      upper_curve.init(num_upper);
      for ( i = 0 ; i < num_arr ; i+=2)
      {
        pnt = vec3d(arr[i], 0.0, arr[i+1]);
        upper_curve.load_pnt( i/2, pnt );
      }
      free(arr);
    }
    xmlNodePtr low_node = xmlGetNode( node, "Lower_Pnts", 0 );

    if ( low_node )
    {
      int num_arr =  xmlGetNumArray( low_node, ',' );
      double* arr = (double*)malloc( num_arr*sizeof(double) );
      xmlExtractDoubleArray( low_node, ',', arr, num_arr );

      int num_lower = num_arr/2;

      lower_curve.init(num_lower);
      for ( i = 0 ; i < num_arr ; i+=2)
      {
        pnt = vec3d(arr[i], 0.0, arr[i+1]);
        lower_curve.load_pnt( i/2, pnt );
      }
      free(arr);
    }
  }

  set_type(temp_type);
}

//==== Write Airfoil File ====//
void Af::write(FILE* file_id)
{
  int i;
  vec3d pnt;

  // Shahab Hasan - 11/30/95 - Eliminated backslashes from following printf.
  // fprintf(file_id, "\/\/**** AIRFOIL ****\/\/\n");
  fprintf(file_id, "//**** AIRFOIL ****//\n");
  fprintf(file_id, "%d				Airfoil Type\n",type);
  fprintf(file_id, "%d				Inverted Flag\n",inverted_flag);
  fprintf(file_id, "%f			Airfoil Camber\n",camber());
  fprintf(file_id, "%f			Camber Loc\n",camber_loc());
  fprintf(file_id, "%f			Thickness\n",thickness());
  fprintf(file_id, "%f			Thickness_Loc\n",thickness_loc());

  fprintf(file_id, "%f			Radius_Le\n",radius_le());
  fprintf(file_id, "%f			Radius_Te\n",radius_te());

  if ( type == NACA_6_SERIES )
  {
  	fprintf(file_id, "%d				Six Series\n",sixser);
    fprintf(file_id, "%f			  Ideal Cl\n",ideal_cl());
    fprintf(file_id, "%f			  A\n",a());
  }

  if ( type == AIRFOIL_FILE )
    {
      fprintf(file_id, "%s			\n", name());
      fprintf(file_id, "%f			Original AF Thickness\n", orig_af_thickness);
      fprintf(file_id, "%f			Radius LE Correct Factor\n", radius_le_correction_factor);
      fprintf(file_id, "%f			Radius TE Correct Factor\n", radius_te_correction_factor);
      int num_upper = upper_curve.get_num_pnts();
      fprintf(file_id, "%d				Num_Pnts_Upper\n",num_upper);
      for (  i = 0 ; i < num_upper ; i++)
         {
           pnt = upper_curve.get_pnt(i);
           fprintf(file_id, "%f  %f    ", pnt.x(), pnt.z());
           if ((i+1)%5 == 0)
             fprintf(file_id, "\n");
         }
       if ( i%5 != 0)
         fprintf(file_id, "\n");
      int num_lower = lower_curve.get_num_pnts();
      fprintf(file_id, "%d				Num_Pnts_Lower\n",num_lower);
      for ( i = 0 ; i < num_lower ; i++)
         {
           pnt = lower_curve.get_pnt(i);
           fprintf(file_id, "%f  %f    ", pnt.x(), pnt.z());
           if ((i+1)%5 == 0)
             fprintf(file_id, "\n");
         }
       if ( i%5 != 0)
         fprintf(file_id, "\n");
    }
}

//==== Write Airfoil Points to Aero File ====//
void Af::write_aero_points(FILE* file_id)
{
  load_curves();

//   double sign = 1.0;
//   if (inverted_flag == TRUE)
//     sign = -1.0;

  int num_upper = upper_curve.get_num_pnts();
  int num_lower = lower_curve.get_num_pnts();

  //==== Write Upper Points =====
  fprintf(file_id, "%d			Number of Upper Surface Points\n",num_upper);

  if (inverted_flag == TRUE)
  {
    for (int i = 0; i < num_lower; i++)
    {
      vec3d pnt = lower_curve.get_pnt(i);
      fprintf(file_id, "%f  %f\n", pnt.x(), -pnt.z());
    }
  }
  else
  {
    for (int i = 0; i < num_upper; i++)
    {
      vec3d pnt = upper_curve.get_pnt(i);
      fprintf(file_id, "%f  %f\n", pnt.x(), pnt.z());
    }
  }

  //===== Write Lower Points =====
  fprintf(file_id, "%d			Number of Lower Surface Points\n",num_lower);

  if (inverted_flag == TRUE)
  {
    for (int i = 0; i < num_upper; i++)
    {
      vec3d pnt = upper_curve.get_pnt(i);
      fprintf(file_id, "%f  %f\n", pnt.x(), -pnt.z());
    }
  }
  else
  {
    for (int i = 0; i < num_lower; i++)
    {
      vec3d pnt = lower_curve.get_pnt(i);
      fprintf(file_id, "%f  %f\n", pnt.x(), pnt.z());
    }
  }
}

//==== Read Airfoil File ====//
void Af::read(FILE* file_id)
{
  int i;
  Stringc line;
  char buff[255];
  int temp_type;
  int version = geom_ptr->get_aircraft_ptr()->get_version();

  fgets(buff, 80, file_id);
  fgets(buff, 80, file_id);  line = buff;

  if (line.search_for_substring("Num") >= 0)
    {
      sscanf(buff, "%d", &num_pnts);
      if (version >= 5) 
      {
        fscanf(file_id, "%d", &inverted_flag);
        fgets(buff, 80, file_id);
      }
      camber.read(file_id);
      camber_loc.read(file_id);
      thickness.read(file_id);
      temp_type = NACA_4_SERIES;
    }
  else
    {
      sscanf(buff, "%d", &temp_type);
      if (version >= 5)
      {
        fscanf(file_id, "%d", &inverted_flag);
        fgets(buff, 80, file_id);
      }
      camber.read(file_id);
      camber_loc.read(file_id);
      thickness.read(file_id);
      thickness_loc.read(file_id);
      radius_le.read(file_id);
      radius_te.read(file_id);

      if ( temp_type ==  NACA_6_SERIES )
      {
        fscanf( file_id, "%d", &sixser );
        fgets(buff, 80, file_id);
        ideal_cl.read(file_id);
        a.read(file_id);
      }

      if ( temp_type == AIRFOIL_FILE )
        {
          fgets(buff, 80, file_id);
          name = buff;
          name.remove_trailing_blanks();
          name.remove_all_substrings('\n');

          vec3d pnt;
          float x, z;
          int num_upper, num_lower;

          fscanf(file_id, "%f",&orig_af_thickness);		fgets(buff, 80, file_id);
          fscanf(file_id, "%f",&radius_le_correction_factor);	fgets(buff, 80, file_id);
          fscanf(file_id, "%f",&radius_te_correction_factor);	fgets(buff, 80, file_id);
          fscanf(file_id, "%d",&num_upper); 			fgets(buff, 80, file_id);
          upper_curve.init(num_upper);
          for ( i = 0 ; i < num_upper ; i++)
            {
              fscanf(file_id, "%f  %f",&x, &z);
              pnt = vec3d(x, 0.0, z);
              upper_curve.load_pnt( i, pnt );
            }
          fgets(buff, 80, file_id);
          fscanf(file_id, "%d",&num_lower); 			fgets(buff, 80, file_id);
          lower_curve.init(num_lower);
          for (  i = 0 ; i < num_lower ; i++)
            {
              fscanf(file_id, "%f  %f",&x, &z);
              pnt = vec3d(x, 0.0, z);
              lower_curve.load_pnt( i, pnt );
            }
          fgets(buff, 80, file_id);
        }
     }
  set_type(temp_type);
}

//===== Generate Airfoil =====//
void Af::generate_airfoil()
{
  switch(type)
    {
      case NACA_4_SERIES:
        generate_4_series();
        break;

      case BICONVEX:
        generate_biconvex();
        break;

      case WEDGE:
        generate_wedge();
        break;

      case AIRFOIL_FILE:
        generate_from_file_curve();
        break;

      case NACA_6_SERIES:
			  load_6_series_curves();
        generate_6_series();
				break;
      
    }

  radius_le = compute_leading_edge_radius();
  curve_loaded_flag = FALSE;

  if (inverted_flag)
    invert_airfoil();

  add_slat_flap();


}

void Af::add_slat_flap()
{
	if ( slat_flag )
	{
		for ( int i = 0 ; i < pnts.dimension() ; i++ )
		{
			if ( pnts[i].x() < slat_chord() )
			{
				if ( slat_shear_flag )
				{
					double x = slat_chord() - pnts[i].x();
					double del_z = x*tan( slat_angle()*DEG_2_RAD );
					pnts[i].set_z( pnts[i].z() + del_z );
				}
				else
				{
					double r = slat_chord() - pnts[i].x();
					double del_x = r - r*cos( slat_angle()*DEG_2_RAD );
					double del_z = r*sin( slat_angle()*DEG_2_RAD );
					pnts[i].set_x( pnts[i].x() + del_x );
					pnts[i].set_z( pnts[i].z() + del_z );
				}
			}
		}
	}
	if ( flap_flag )
	{
		for ( int i = 0 ; i < pnts.dimension() ; i++ )
		{
			if ( pnts[i].x() > (1.0 - flap_chord()) )
			{
				if ( flap_shear_flag )
				{
					double x = pnts[i].x() - (1.0 - flap_chord() );
					double del_z = x*tan( flap_angle()*DEG_2_RAD );
					pnts[i].set_z( pnts[i].z() + del_z );
				}
				else
				{
					double r = pnts[i].x() - (1.0 - flap_chord() );
					double del_x = r*cos( flap_angle()*DEG_2_RAD ) - r;
					double del_z = r*sin( flap_angle()*DEG_2_RAD );
					pnts[i].set_x( pnts[i].x() + del_x );
					pnts[i].set_z( pnts[i].z() + del_z );

				}
			}
		}
	}
}

//===== Generate 4_Series Airfoil =====//
void Af::generate_4_series()
{
  int half_pnts;
  float x, xu, zu, zt, xl, zl, zc, theta;
 
  //==== Initialize Array For Points ====//
  int npnts = num_pnts;
  pnts.init(npnts);

  //==== Generate Airfoil ====//
  half_pnts = npnts/2;

  for ( int i = 1 ; i < half_pnts ; i++)
    {
      //==== COS**2 Distribution of Points ====//
//      x = 0.5*(1.0 - cos( (float)i/(float)half_pnts*M_PI ));
//      x = x*x;
            
      //==== More Evenly Spaced Distribution of Points -DJK ====//
      x = (float)i / (float)half_pnts;
      x = x*sqrt(x);

      //==== Compute Camber Line and Thickness ====//
      compute_zc_zt_theta( x, zc, zt, theta);

      //==== Compute Upper Surface Points ====//
      xu = x  - zt*sin(theta); 
      zu = zc + zt*cos(theta);
      pnts[half_pnts-i] = vec3d(xu, 0.0, zu);

      //==== Compute Lower Surface Points ====//
      xl = x  + zt*sin(theta);
      zl = zc - zt*cos(theta);
      pnts[half_pnts+i] = vec3d(xl, 0.0, zl);
    }

  pnts[0]          = vec3d(1.0, 0.0, 0.0);
  pnts[npnts-1]    = vec3d(1.0, 0.0, 0.0);
  pnts[half_pnts]  = vec3d(0.0, 0.0, 0.0);

  
}

//===== Generate Biconvex Airfoil =====//
void Af::generate_biconvex()
{
  int half_pnts;
  double x, xu, zu, xl, zl;
 
  //==== Initialize Array For Points ====//
  int npnts = num_pnts;
  pnts.init(npnts);

  half_pnts = npnts/2;
  for ( int i = 1 ; i < half_pnts ; i++)
    {
      //==== COS**2 Distribution of Points ====//
      x = (float)i/(float)half_pnts;
      //==== Compute Upper Surface Points ====//
      xu = x;
      zu = 2.0*thickness()*x*(1.0 - x);
      pnts[half_pnts-i] = vec3d(xu, 0.0, zu);

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      pnts[half_pnts+i] = vec3d(xl, 0.0, zl);
    }

  pnts[0]         = vec3d(1.0, 0.0, 0.0);
  pnts[npnts-1]   = vec3d(1.0, 0.0, 0.0);
  pnts[half_pnts] = vec3d(0.0, 0.0, 0.0);

  
}

//===== Generate Wedge Airfoil =====//
void Af::generate_wedge()
{
  int i;
  int half_pnts, apex;
  double x, xu, zu, xl, zl, a, b;
 
  //==== Initialize Array For Points ====//
  int npnts = num_pnts;
  pnts.init(npnts);

  half_pnts = npnts/2;
  apex = (int)( (float)half_pnts*thickness_loc()) + 1;
  if ( half_pnts == apex )
    apex--;

  //==== Compute Front Side Points ====//
  for ( i = 1 ; i < apex ; i++)
    {
      //==== Linear Distribution of Points ====//
      x = (float)i / (float)apex * thickness_loc();

      //==== Compute Upper Surface Points ====//
      a = 0.5*thickness()/thickness_loc();
      xu = x;
      zu = a*xu;
      pnts[half_pnts-i] = vec3d(xu, 0.0, zu);

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      pnts[half_pnts+i] = vec3d(xl, 0.0, zl);
    }

  //==== Compute Back Side Points ====//
  for ( i = apex ; i < half_pnts ; i++)
    {
      //==== Linear Distribution of Points ====//
      x = (float) (i - apex) / (float) (half_pnts - apex) * (1.0 - thickness_loc()) + thickness_loc();

      //==== Compute Upper Surface Points ====//
      a = -0.5*thickness()/(1.0 - thickness_loc());
      b = -a;
      xu = x;
      zu = a*xu + b;
      pnts[half_pnts-i] = vec3d(xu, 0.0, zu);

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      pnts[half_pnts+i] = vec3d(xl, 0.0, zl);
    }

  pnts[0]          = vec3d(1.0, 0.0, 0.0);
  pnts[npnts-1]    = vec3d(1.0, 0.0, 0.0);
  pnts[half_pnts]  = vec3d(0.0, 0.0, 0.0);
  
}
//===== Generate From File Curve =====//
void Af::generate_6_series()
{
  int i;
  float u;

  //==== Initialize Array For Points ====//
  int npnts = num_pnts;
  pnts.init(npnts);

  //==== Generate Airfoil ====//
  int half_pnts = npnts/2;

  for (  i = 0 ; i < half_pnts+1 ; i++)
    {
      //==== X*X Distribution of Points ====//
      u = (float)i / (float)half_pnts;
      u = u*sqrt(u);

      //==== More Evenly Spaced Distribution of Points -DJK ====//
      u = (float)i / (float)half_pnts;
      u = u*sqrt(u);

      //==== Compute Upper Surface Points ====//
      pnts[half_pnts-i] = upper_curve.comp_pnt_per_length(u);

      //==== Compute Lower Surface Points ====//
      pnts[half_pnts+i] = lower_curve.comp_pnt_per_length(u);
    }
  pnts[0]          = vec3d(1.0, 0.0, 0.0);
  pnts[npnts-1]    = vec3d(1.0, 0.0, 0.0);
  pnts[half_pnts]  = vec3d(0.0, 0.0, 0.0);

}

//===== Compute Radius of Leading Edge to Chord =====//
float Af::compute_leading_edge_radius()
{

  //-----------------------------------------------------------------------------
  // Shahab Hasan - 11/30/95 - Introduced a local variable "answer" to be used
  //                           in the switch statement instead of multiple
  //                           returns.  "answer" is then returned at the and
  //                           of the function.  This elimates compiler warnings.
  //-----------------------------------------------------------------------------
  float	answer;

  switch(type)
    {
      case NACA_4_SERIES:
        answer = (float)(1.1019*thickness()*thickness());
        break;

      case BICONVEX:
        answer = 0.0;
        break;

      case WEDGE:
        answer = 0.0;
        break;

      case AIRFOIL_FILE:
        answer = compute_radius_le_af_curves()*radius_le_correction_factor;
        break;

      case NACA_6_SERIES:
        answer = (float)(1.1019*thickness()*thickness());
//        answer = compute_radius_le_af_curves()*radius_le_correction_factor;
        break;
    }

  return answer;

}

//===== Get Delta Y (actually Z) of Leading Edge: thickness@ 6%c - thickness@ .15%c =====//
float Af::get_delta_y_le()
{

  //-----------------------------------------------------------------------------
  // Shahab Hasan - 11/30/95 - Introduced a local variable "answer" to be used
  //                           in the switch statement instead of multiple
  //                           returns.  "answer" is then returned at the and
  //                           of the function.  This elimates compiler warnings.
  //-----------------------------------------------------------------------------
  float	answer;

  switch(type)
    {
      case NACA_4_SERIES:
        float zt1,zt2,zc,theta;
        compute_zc_zt_theta( 0.06f, zc, zt2, theta);
        compute_zc_zt_theta( 0.0015f, zc, zt1, theta);
        answer = 2.0f*(zt2 - zt1);
        break;

     case BICONVEX:
        float thick1, thick2;
        thick1 = (float)(4.0*thickness()*0.06*(1.0 - 0.06));
        thick2 = (float)(4.0*thickness()*0.0015*(1.0 - 0.0015));
        answer = thick1 - thick2;
        break;

      case WEDGE:
        answer = (float)(thickness()/thickness_loc()*(0.06 - 0.0015));
        break;

      case AIRFOIL_FILE:
        answer = (float)delta_y_le();
        break;

      case NACA_6_SERIES:
        compute_zc_zt_theta( 0.06f, zc, zt2, theta);
        compute_zc_zt_theta( 0.0015f, zc, zt1, theta);
        answer = (float)(2.0*(zt2 - zt1));
        break;

    }

  return answer;

}

//===== Compute Camber Line and Thickness: 4_Series Airfoil =====//
void Af::compute_zc_zt_theta( float x, float& zc, float& zt, float& theta)
{
  double xx, xo, xoxo, zo;

  xx = x*x;
  zt = (float)( (thickness()/0.20)*
       (0.2969*sqrt(x) - 0.1260*x - 0.3516*xx + 0.2843*x*xx - 0.1015*xx*xx));

  if ( camber() <= 0.0 || camber_loc() <= 0.0 || camber_loc() >= 1.0 )
    {
      zc = 0.0;
      theta = 0.0;
    }
  else
    {
      if ( x < camber_loc())
        zc = (float)((camber()/(camber_loc()*camber_loc()))*(2.0*camber_loc()*x - xx));
      else
        zc = (float)((camber()/((1.0 - camber_loc())*(1.0 - camber_loc()))) *
             ( 1.0 - 2.0*camber_loc() + 2.0*camber_loc()*x - xx));

      xo = x + 0.00001;
      xoxo = xo*xo;

      if ( xo < camber_loc())
        zo = (camber()/(camber_loc()*camber_loc()))*(2.0*camber_loc()*xo - xoxo);
      else
        zo = (camber()/((1.0 - camber_loc())*(1.0 - camber_loc()))) *
             ( 1.0 - 2.0*camber_loc() + 2.0*camber_loc()*xo - xoxo);

      theta = atan( ((float)zo - zc)/0.00001f );
    }

}

//===== Load Airfoil Name =====//
void Af::load_name()
{
  switch(type)
    {
      case NACA_4_SERIES:
        load_name_4_series();
        break;

      case BICONVEX:
        load_name_biconvex();
        break;

      case WEDGE:
        load_name_wedge();
        break;

      case AIRFOIL_FILE:
        break;

      case NACA_6_SERIES:
        load_name_6_series();
        break;

    }

}

//===== Load Name And Number of 4 Series =====//
void Af::load_name_4_series()
{
  int icam     = int(camber()*100.0f + 0.5f);
  int icam_loc = int(camber_loc()*10.0f + 0.5f);
  int ithick   = int(thickness()*100.0f + 0.5f);

  if (icam == 0) icam_loc = 0;
 
  char str[255];
  if (ithick < 10)
    sprintf(str, "NACA %d%d0%d", icam, icam_loc, ithick);
  else
    sprintf(str, "NACA %d%d%d", icam, icam_loc, ithick);

  name = str;
  
}

//===== Load Name And Number of 4 Series =====//
void Af::load_name_6_series()
{
  int ithick   = int(thickness()*100.0 + 0.5f);
  int icl = int(ideal_cl()*10.0 + 0.5f);
  int ia = int(a()*10.0 + 0.5f);

  char str[255];

  if ( sixser > 0 )
  {
    if (ithick < 10)
      sprintf(str, "NACA %d%d0%d  a=0.%d", sixser, icl, ithick, ia);
    else
      sprintf(str, "NACA %d%d%d  a=0.%d", sixser, icl, ithick, ia);
  }
  else
  {
    if (ithick < 10)
      sprintf(str, "NACA %dA%d0%d  a=0.%d", -sixser, icl, ithick, ia);
    else
      sprintf(str, "NACA %dA%d%d  a=0.%d", -sixser, icl, ithick, ia);
  }
  name = str;

}

//===== Load Name of Biconvex Airfoil =====//
void Af::load_name_biconvex()
{
  int ithick   = int(thickness()*100.0 + 0.5f);
 
  char str[255];

  sprintf(str, "%d%% BICONVEX", ithick);

  name = str;
  
}

//===== Load Name of Wedge Airfoil =====//
void Af::load_name_wedge()
{
  int ithick   = int(thickness()*100.0 + 0.5f);
 
  char str[255];

  sprintf(str, "%d%% WEDGE", ithick);

  name = str;
  
}

//===== Set Number of Points Per X-Sec =====//
void Af::set_num_pnts(int num_pnts_in)
{
  if ( num_pnts_in != num_pnts )
    {
      num_pnts = num_pnts_in;
      generate_airfoil();
    }
}

//===== Set Type =====//
void Af::set_type(int type_in)
{
//  if ( type == type_in ) return;

  type = type_in;

	//==== Turn Of Everything ====//
	camber.deactivate();
	camber_loc.deactivate();
	thickness_loc.deactivate();
	ideal_cl.deactivate();
	a.deactivate();

  switch(type)
    {
      case NACA_4_SERIES:
  			camber.activate();
  			camber_loc.activate();
        break;

      case BICONVEX:
        break;

      case WEDGE:
        thickness_loc.activate();
        break;

      case AIRFOIL_FILE:
        break;

      case NACA_6_SERIES:      //jrg more...
				ideal_cl.activate();
				a.activate();
        break;

    }

  generate_airfoil();
  load_name();
}

//===== Set Six Series =====//
void Af::set_sixser( int ser )
{
  sixser = ser;
  generate_airfoil();
  load_name();
}

//===== Read AF File =====//
int Af::read_af_file( Stringc file_name )
{
  Stringc line;
  char buff[255];
  int thickness_read_flag = 0;
  int radius_le_read_flag = 0;
  int radius_te_read_flag = 0;
  
  /* --- Open file --- */
  FILE* file_id = fopen(file_name, "r");

  if (file_id == (FILE *)NULL) return(0);

  fgets(buff, 80, file_id);  line = buff;

  if (line.search_for_substring("AIRFOIL FILE") < 0)
  {
		int ok = readSeligAirfoil( file_id );	

		if ( ok )
		{
    	name = buff;
    	name.remove_trailing_blanks();
   		name.remove_all_substrings('\n');
   		name.remove_all_substrings('\r');
		}	
  }
 	else
	{
		int ok = readVspAirfoil( file_id,
			&thickness_read_flag, &radius_le_read_flag, &radius_te_read_flag );
	}

  fclose( file_id );

  //==== Set Type and Deactivate Parameters ====//
  set_type( AIRFOIL_FILE );

  //==== Check if  Thickness was in File - Compute if not ====//
  if ( !thickness_read_flag )
    thickness = compute_thickness_af_curves();

  //==== Set Base Thickness Value ====//
  orig_af_thickness = (float)thickness();

  //==== Check if  Radius LE was in File - Compute if not ====//
  if ( !radius_le_read_flag )
    {
      radius_le = compute_radius_le_af_curves();
      radius_le_correction_factor = 1.0;
    }
  else
    {
      radius_le_correction_factor = (float)(radius_le()/compute_radius_le_af_curves());
    }

  //==== Check if  Radius TE was in File - Compute if not ====//
  if ( !radius_te_read_flag )
    {
      radius_te = 0.0;
    }

  upper_curve.comp_tans();
  lower_curve.comp_tans();

  vec3d leading_edge_tan = vec3d(0.0, 0.0, 1.0);

  double tan_mag = upper_curve.get_tan(0).mag();
  upper_curve.load_tan( 0, leading_edge_tan*tan_mag);

  tan_mag = -(lower_curve.get_tan(0).mag());
  lower_curve.load_tan( 0, leading_edge_tan*tan_mag);

  generate_from_file_curve();

  return(1);

}

int Af::readSeligAirfoil( FILE* file_id )
{
  int i;
  Stringc line;
  char buff[255];
  float x, z;
	sdyn_array< float > xvec;
	sdyn_array< float > zvec;

  int more_data_flag = 1;
  while (more_data_flag)
    {
      more_data_flag = 0;
      if ( fgets(buff, 80, file_id) )
        {
          more_data_flag = 1;
          line = buff;
        }
			
			if ( more_data_flag )
			{
				x = z = 100000.0;
				sscanf(buff, "%f %f", &x, &z);
				if ( x >= 0.0 && x <= 1.0 && z >= -1.0 && z <= 1.0 )
				{
					xvec.append( x );
					zvec.append( z );
				}
				else
				{
					more_data_flag = 0;
				}									
			}
		}
//	fclose(file_id);


	int totalPnts = xvec.dimension();

	if ( totalPnts < 5 )
		return 0;

	//==== Find Leading Edge Index ====//
	int leInd = 0;
	float smallestX = 1.0e06;
	for ( i = 0 ; i < totalPnts ; i++ )
	{
		if ( xvec[i] < smallestX )
		{
			smallestX = xvec[i];
			leInd = i;
		}
// 		if ( fabs(xvec[i]) < 0.000001 )			// Not reliable....
//			leInd = i;
	}
	
	//==== Not Enough Pnts ====//
	int num_pnts_lower = totalPnts - leInd;
	int num_pnts_upper = leInd + 1;


	if ( num_pnts_lower < 3 || num_pnts_upper < 3 )
		return 0;

  upper_curve.init(num_pnts_upper);
  lower_curve.init(num_pnts_lower);

	//==== Load Em Up ====//
	int vind = 0;
	for ( i = leInd ; i >= 0 ; i-- )
	{
  	upper_curve.load_pnt(  vind, vec3d(xvec[i], 0.0, zvec[i]));
		vind++;
	}

  vind = 0;
	for ( i = leInd ; i < totalPnts ; i++ )
	{
  	lower_curve.load_pnt(  vind, vec3d(xvec[i], 0.0, zvec[i]));
		vind++;
	}
  //==== Close Trailing Edge - Set Last Points ====//
  vec3d last_pnt = upper_curve.get_pnt(num_pnts_upper-1) +
                   lower_curve.get_pnt(num_pnts_lower-1);
  upper_curve.load_pnt(num_pnts_upper-1, last_pnt*0.5);
  lower_curve.load_pnt(num_pnts_lower-1, last_pnt*0.5);		

	return 1;

}


int Af::readVspAirfoil( FILE* file_id,
		int *thickness_read_flag, int *radius_le_read_flag, int *radius_te_read_flag  )
{
   int i;
   Stringc line;
   char buff[255];

   fgets(buff, 80, file_id);
   name = buff;
   name.remove_trailing_blanks();
   name.remove_all_substrings('\n');

   int sym_flag;
   int num_pnts_upper, num_pnts_lower;
   float x, z;

   fscanf(file_id, "%d", &sym_flag);
   fgets(buff, 80, file_id);

   fscanf(file_id, "%d", &num_pnts_upper);
   fgets(buff, 80, file_id);

    upper_curve.init(num_pnts_upper);
    if (sym_flag)
     {
       num_pnts_lower = num_pnts_upper;
       lower_curve.init(num_pnts_lower);
     }
   else
     {
       fscanf(file_id, "%d", &num_pnts_lower);
       fgets(buff, 80, file_id);
       lower_curve.init(num_pnts_lower);
     }

   for (i = 0 ; i < num_pnts_upper ; i++)
     {
       fscanf(file_id, "%f %f", &x, &z);
       fgets(buff, 80, file_id);

       upper_curve.load_pnt(  i, vec3d(x, 0.0, z));

       if ( sym_flag )
         {
           lower_curve.load_pnt(  i, vec3d(x, 0.0, -z) );
         }
     }
   fgets(buff, 80, file_id);


  if ( !sym_flag )
    {
      for ( i = 0 ; i < num_pnts_lower ; i++)
        {
          fscanf(file_id, "%f %f", &x, &z);
          fgets(buff, 80, file_id);
          lower_curve.load_pnt( i, vec3d(x, 0.0, z) );
        }
      fgets(buff, 80, file_id);
    }

  //==== Close Trailing Edge - Set Last Points ====//
  vec3d last_pnt = upper_curve.get_pnt(num_pnts_upper-1) + 
                   lower_curve.get_pnt(num_pnts_lower-1);
  upper_curve.load_pnt(num_pnts_upper-1, last_pnt*0.5);
  lower_curve.load_pnt(num_pnts_lower-1, last_pnt*0.5);

  float fdum;  
  int more_data_flag = 1;
  while (more_data_flag)
    {
      more_data_flag = 0;
      if ( fgets(buff, 80, file_id) )
        {
          more_data_flag = 1;
          line = buff;
        }
      else
        {
          line = " ";
        }

      if (line.search_for_substring("Radius") >= 0)
        {
          sscanf(buff, "%f", &fdum);
         if (line.search_for_substring("Trailing") >= 0)
            {
              radius_te = fdum;
              *radius_te_read_flag = 1;
            }
          else
            {
              radius_le = fdum;
              *radius_le_read_flag = 1;
            }
        }
      if (line.search_for_substring("Thick") >= 0)
        {
          sscanf(buff, "%f", &fdum);
          if (line.search_for_substring("Location") >= 0)
            {
              thickness_loc = fdum;
            }
          else
            {
              *thickness_read_flag = 1;
              thickness = fdum;
            }
        }
      if (line.search_for_substring("Camber") >= 0)
        {
          sscanf(buff, "%f", &fdum);
          if (line.search_for_substring("Location") >= 0)
            {
              camber_loc = fdum;
            }
          else
            {
              camber = fdum;
            }
        }
      if (line.search_for_substring("Delta") >= 0)
        {
          sscanf(buff, "%f", &fdum);
          delta_y_le = fdum;
        }
    }

  fclose(file_id);

	return 1;

}

//===== Generate From File Curve =====//
float Af::compute_thickness_af_curves()
{
  float u;
  vec3d pnt_u, pnt_l;
  double max_thickness = 0.0;
  for ( int i = 0 ; i < 11 ; i++)
    {
      u = 0.05f + (float)i*0.05f;
      pnt_u = upper_curve.comp_pnt_per_length(u);
      pnt_l = lower_curve.comp_pnt_per_length(u);

      double dist_tween = dist( pnt_u, pnt_l );
      if ( dist_tween > max_thickness )
        max_thickness = dist_tween;
    }
  return( (float)max_thickness );
}

//===== Compute Radius Le From File Curve =====//
float Af::compute_radius_le_af_curves()
{
  double thick_scale = thickness()/orig_af_thickness;
  vec3d p0 = lower_curve.get_pnt(1)*thick_scale;
  vec3d p1 = upper_curve.get_pnt(1)*thick_scale;
  return((float)radius_of_circle(p0, upper_curve.get_pnt(0), p1  ));
}

//===== Generate From File Curve =====//
void Af::generate_from_file_curve()
{
  int i;
  float u;

  //==== Initialize Array For Points ====//
  int npnts = num_pnts;
  pnts.init(npnts);

  //==== Generate Airfoil ====//
  int half_pnts = npnts/2;

  for (  i = 0 ; i < half_pnts+1 ; i++)
    {
      //==== X*X Distribution of Points ====//
      u = (float)i / (float)half_pnts;
      u = u*sqrt(u);
//      u = u*u;

      //==== More Evenly Spaced Distribution of Points -DJK ====//
      u = (float)i / (float)half_pnts;
      u = u*sqrt(u);

      //==== Compute Upper Surface Points ====//
      pnts[half_pnts-i] = upper_curve.comp_pnt_per_length(u);

      //==== Compute Lower Surface Points ====//
      pnts[half_pnts+i] = lower_curve.comp_pnt_per_length(u);
    }

  //==== Scale Airfoil Thickness ====//
  double z_avg;
  double thick_scale = thickness()/orig_af_thickness;

  for ( i = 0 ; i < half_pnts+1 ; i++)
    {
      z_avg = 0.5*(pnts[half_pnts-i].z() + pnts[half_pnts+i].z());
      pnts[half_pnts-i].set_z( (pnts[half_pnts-i].z() - z_avg)*thick_scale + z_avg );
      pnts[half_pnts+i].set_z( (pnts[half_pnts+i].z() - z_avg)*thick_scale + z_avg );
    }      
 }

//===== Load Hermite Curves =====//
void Af::load_curves()
{
  if (!curve_loaded_flag)
  {
    switch(type)
      {
        case NACA_4_SERIES:
          load_4_series_curves();
          break;

        case BICONVEX:
          load_biconvex_curves();
          break;

        case WEDGE:
          load_wedge_curves();
          break;

        case AIRFOIL_FILE:
          break;

        case NACA_6_SERIES:
					load_6_series_curves();
          break;

      }
  curve_loaded_flag = TRUE;
  }
}

//===== Load 4_series Upper & Lower Hermite Curves =====//
void Af::load_4_series_curves()
{
  float x, xu, zu, zt, xl, zl, zc, theta;
 
  //==== Initialize Array For Points ====//
  int half_pnts = num_pnts/2;
  upper_curve.init(half_pnts+1);
  lower_curve.init(half_pnts+1);

  //==== Generate Airfoil ====//
  for ( int i = 1 ; i < half_pnts ; i++ )
    {
      //==== COS**2 Distribution of Points ====//
//      x = 0.5*(1.0 - cos( (float)i/(float)half_pnts*M_PI ));
//      x = x*x;
            
      //==== More Evenly Spaced Distribution of Points -DJK ====//
      x = (float)i / (float)half_pnts;
      x = x*sqrt(x);

      //==== Compute Camber Line and Thickness ====//
      compute_zc_zt_theta( x, zc, zt, theta);

      //==== Compute Upper Surface Points ====//
      xu = x  - zt*sin(theta); 
      zu = zc + zt*cos(theta);
      upper_curve.load_pnt( i, vec3d(xu, 0.0, zu) );

      //==== Compute Lower Surface Points ====//
      xl = x  + zt*sin(theta);
      zl = zc - zt*cos(theta);
      lower_curve.load_pnt( i, vec3d(xl, 0.0, zl) );
    }

  upper_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  upper_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );
  lower_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  lower_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );

  upper_curve.comp_tans();
  lower_curve.comp_tans();

  vec3d leading_edge_tan = vec3d(0.0, 0.0, 1.0);

  double tan_mag = upper_curve.get_tan(0).mag();
  upper_curve.load_tan( 0, leading_edge_tan*tan_mag);

  tan_mag = -(lower_curve.get_tan(0).mag());
  lower_curve.load_tan( 0, leading_edge_tan*tan_mag);

}

//===== Load biconvex Upper & Lower Hermite Curves =====//
void Af::load_biconvex_curves()
{
  double x, xu, zu, xl, zl;
 
  //==== Initialize Array For Points ====//
  int half_pnts = num_pnts/2;
  upper_curve.init(half_pnts+1);
  lower_curve.init(half_pnts+1);

  for ( int i = 1 ; i < half_pnts ; i++)
    {
      //==== COS**2 Distribution of Points ====//
//      x = 0.5*(1.0 - cos( (float)i/(float)half_pnts*M_PI ));
//      x = x*x;
      x = (float)i/(float)half_pnts;
      //==== Compute Upper Surface Points ====//
      xu = x;
      zu = 2.0*thickness()*x*(1.0 - x);
      upper_curve.load_pnt( i, vec3d(xu, 0.0, zu) );

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      lower_curve.load_pnt( i, vec3d(xl, 0.0, zl) );
    }

  upper_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  upper_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );
  lower_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  lower_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );

  upper_curve.comp_tans();
  lower_curve.comp_tans();

}

//===== Load Wedge Upper & Lower Hermite Curves =====//
void Af::load_wedge_curves()
{
  int i;
  int apex;
  double x, xu, zu, xl, zl, a, b;
 
  //==== Initialize Array For Points ====//
  int half_pnts = num_pnts/2;
  upper_curve.init(half_pnts+1);
  lower_curve.init(half_pnts+1);

  half_pnts = num_pnts/2;
  apex = (int)( (float)half_pnts*thickness_loc()) + 1;
  if ( half_pnts == apex )
    apex--;

  //==== Compute Front Side Points ====//
  for (  i = 1 ; i < apex ; i++)
    {
      //==== Linear Distribution of Points ====//
      x = (float)i / (float)apex * thickness_loc();

      //==== Compute Upper Surface Points ====//
      a = 0.5*thickness()/thickness_loc();
      xu = x;
      zu = a*xu;
      upper_curve.load_pnt( i, vec3d(xu, 0.0, zu) );

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      lower_curve.load_pnt( i, vec3d(xl, 0.0, zl) );
    }

  //==== Compute Back Side Points ====//
  for ( i = apex ; i < half_pnts ; i++)
    {
      //==== Linear Distribution of Points ====//
      x = (float) (i - apex) / (float) (half_pnts - apex) * (1.0 - thickness_loc()) + thickness_loc();

      //==== Compute Upper Surface Points ====//
      a = -0.5*thickness()/(1.0 - thickness_loc());
      b = -a;
      xu = x;
      zu = a*xu + b;
      upper_curve.load_pnt( i, vec3d(xu, 0.0, zu) );

      //==== Compute Lower Surface Points ====//
      xl = xu;
      zl = - zu;
      lower_curve.load_pnt( i, vec3d(xl, 0.0, zl) );
    }

  upper_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  upper_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );
  lower_curve.load_pnt(0, vec3d(0.0, 0.0, 0.0) );
  lower_curve.load_pnt(half_pnts, vec3d(1.0, 0.0, 0.0) );

  upper_curve.comp_tans();
  lower_curve.comp_tans();

}

//===== Run Six Series Code =====//
void Af::load_6_series_curves()
{
  int i;

  //==== Run Six Series Fortran Generation Code ====//
  float cli = (float)ideal_cl();
  float ta = (float)a();
  float toc = (float)thickness();
	sixseries_( &sixser, &toc, &cli, &ta );

  int num_pnts_upper = sixpnts_.nmu;
  int num_pnts_lower = sixpnts_.nml;

  upper_curve.init(num_pnts_upper);
  for (i = 0 ; i < num_pnts_upper ; i++)
    {
      float x = sixpnts_.xxu[i];
      float z = sixpnts_.yyu[i];
      upper_curve.load_pnt(  i, vec3d(x, 0.0, z));
    }

  lower_curve.init(num_pnts_lower);
  for (i = 0 ; i < num_pnts_lower ; i++)
    {
      float x = sixpnts_.xxl[i];
      float z = sixpnts_.yyl[i];
      lower_curve.load_pnt(  i, vec3d(x, 0.0, z));
    }

  //==== Close Trailing Edge - Set Last Points ====//
  vec3d last_pnt = upper_curve.get_pnt(num_pnts_upper-1) +
                   lower_curve.get_pnt(num_pnts_lower-1);
  upper_curve.load_pnt(num_pnts_upper-1, last_pnt*0.5);
  lower_curve.load_pnt(num_pnts_lower-1, last_pnt*0.5);


  upper_curve.comp_tans();
  lower_curve.comp_tans();

  vec3d leading_edge_tan = vec3d(0.0, 0.0, 1.0);

  float tan_mag = (float)upper_curve.get_tan(0).mag();
  upper_curve.load_tan( 0, leading_edge_tan*tan_mag);

  tan_mag = -(float)(lower_curve.get_tan(0).mag());
  lower_curve.load_tan( 0, leading_edge_tan*tan_mag);

}

//===== End-Cap  =====//
vec3d Af::get_end_cap(int index)
{
  int mirror = num_pnts-1-index;
  vec3d new_pnt = (pnts[index] + pnts[mirror])*0.5;
  
  return(new_pnt);
}

//===== Rounded End-Cap  =====//
vec3d Af::get_rounded_end_cap(int index)
{
	//==== BiConvex ====//
	int half_pnts = num_pnts/2;

	if ( index == 0 || index == num_pnts-1 )
		return vec3d( 1.0, 0.0, 0.0 );
	else if ( index == half_pnts )
		return vec3d( 0.0, 0.0, 0.0 );

	int mirror = num_pnts-1-index;
	vec3d new_pnt = (pnts[index] + pnts[mirror])*0.5;

	//double x = new_pnt[0];
	//double yu = 2.0*thickness()*x*(1.0 - x);

	double yu = dist( pnts[index], pnts[mirror] )*0.5;

	new_pnt[1] = yu;

	return new_pnt;
}

//===== Invert Airfoil =====//
void Af::invert_airfoil()
{
  int i;
  array <double> z;
  z.init(num_pnts);

  //===== Switch Upper and Lower Z values =====
  for ( i = 0; i < num_pnts; i++)
  {
    z[i] = pnts[i].z();
  }

  for (i = 0; i < num_pnts; i++)
  {
    pnts[num_pnts-1-i].set_z( -z[i] ); 
  }
}

void Af::draw()
{
	int i;
	//==== Draw Grid ====//
	float gridSize = 0.1f;

	glLineWidth(1.0);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin( GL_LINES );
	for ( i = 0 ; i < 41 ; i++ )
	{
		if ( i == 20 )
			glColor3f(0.8f, 0.8f, 0.8f);
		else
			glColor3f(0.9f, 0.9f, 0.9f);

		glVertex2f( gridSize*(float)i - 20.0f*gridSize, -20.0f*gridSize );
		glVertex2f( gridSize*(float)i - 20.0f*gridSize,  20.0f*gridSize );
		glVertex2f( -20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
		glVertex2f(  20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
	}
	glEnd();

	glPushMatrix();

	glColor3f(1.0, 0.0, 0.0);
	glBegin( GL_LINE_STRIP );
    for (  i = 0 ; i < num_pnts ; i++)
    {
		glVertex2d( pnts[i].x() - 0.5, pnts[i].z() );
	}
	glEnd();

	glPopMatrix();
}


void Af::init_script(Stringc s)
{
	char str[255];

	sprintf(str, "%s camber", s());
	get_camber()->set_script(Stringc(str), 0);

	sprintf(str, "%s camberloc", s());
	get_camber_loc()->set_script(Stringc(str), 0);
	
	sprintf(str, "%s thick", s());
	get_thickness()->set_script(Stringc(str), 0);
	
	sprintf(str, "%s thickloc", s());
	get_thickness_loc()->set_script(Stringc(str), 0);
	
	sprintf(str, "%s idealcl", s());
	get_ideal_cl()->set_script(Stringc(str), 0);
	
	sprintf(str, "%s a", s());
	get_a()->set_script(Stringc(str), 0);
}

void Af::sterilize_parms()
{
	camber.clear_dev_list();
	camber_loc.clear_dev_list();
	thickness.clear_dev_list();
	thickness_loc.clear_dev_list();
	radius_le.clear_dev_list();
	radius_te.clear_dev_list();
	delta_y_le.clear_dev_list();
	a.clear_dev_list();
	ideal_cl.clear_dev_list();
	slat_chord.clear_dev_list();
	slat_angle.clear_dev_list();
	flap_chord.clear_dev_list();
	flap_angle.clear_dev_list();
}

