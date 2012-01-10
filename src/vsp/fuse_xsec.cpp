//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Fusealge Cross-Section
//  
// 
//   J.R. Gloudemans - 8/2/93
//   Sterling Software
//
//
//******************************************************************************

#include "fuse_geom.h"
#include <math.h>
#include "fuse_xsec.h"
#include "util.h"
#include "defines.h"
#include "bezier_curve.h"

//===== Constructor  =====//
fuse_xsec::fuse_xsec(Geom* geom_ptr_in)
{

  geom_ptr = geom_ptr_in;
  	
  edit_crv[0].set_geom( this );
  edit_crv[1].set_geom( this );


  mlType = OML;    				// Mold Line Type

  imlFlag = 0;		
  interpThickFlag = 0;

  updateFuse.initialize(geom_ptr, UPD_FUSE_XSEC, "Update Fuse", 0.0 );

  topThick.initialize(this, UPD_FUSE_XSEC, "Top_Thick", 0.5 );
  topThick.set_lower_upper(0.0, 1000000.0);
  botThick.initialize(this, UPD_FUSE_XSEC, "Bot_Thick", 0.5 );
  botThick.set_lower_upper( 0.0, 1000000.0);
  sideThick.initialize(this, UPD_FUSE_XSEC, "Side_Thick", 0.5 );
  sideThick.set_lower_upper( 0.0, 1000000.0);

  imlXOff = 0.0;
  imlZOff = 0.0;
  actTopThick  = topThick();
  actBotThick  = botThick();
  actSideThick = sideThick();

//  pntSpaceType = PNT_SPACE_FIXED;
  pntSpaceType =PNT_SPACE_UNIFORM;

  drawScaleFactor   = 1.0;
  rollerScaleFactor = 2.0;
  setDrawScaleFactor( rollerScaleFactor );

  num_pnts = 33;

  z_offset.initialize(this, UPD_FUSE_XSEC, "Z_Offset", 0.0 );
  z_offset.set_lower_upper(-1000000.0, 1000000.0);
  z_offset.set_script("fuse_xsec zoffset", 0);

  location.initialize(this, UPD_FUSE_XSEC, "Location", 0.0 );
  location.set_lower_upper(0.0, 1.0);
  location.set_script("fuse_xsec location", 0);

  //==== Profile Stuff ====//
  profileTanStr1.initialize(this, UPD_FUSE_XSEC, "ProfileTanStr1", 0.25 );
  profileTanStr1.set_lower_upper(0.0, 1.0);
  profileTanStr1.set_script("fuse_xsec profiletanstr1", 0); 

  profileTanStr2.initialize(this, UPD_FUSE_XSEC, "ProfileTanStr2", 0.25 );
  profileTanStr2.set_lower_upper(0.0, 1.0);
  profileTanStr2.set_script("fuse_xsec profiletanstr2", 0); 

  profileTanAng.initialize(this, UPD_FUSE_XSEC, "ProfileTanAng", 0.0 );
  profileTanAng.set_lower_upper(-10.0, 10.0);
  profileTanAng.set_script("fuse_xsec profiletanang", 0); 

  for ( int i = 0 ; i < 2 ; i++ )
  {
  	top_crv[i].init(2);
  	bot_crv[i].init(2);

  	pnts[i].init(num_pnts);

	height[i].initialize(this, UPD_FUSE_XSEC, "Height", 3.0 );
    height[i].set_lower_upper(0.0, 1000000.0);
	height[i].set_script("fuse_xsec height", 0);

	width[i].initialize(this, UPD_FUSE_XSEC, "Width", 2.5 );
    width[i].set_lower_upper(0.0, 1000000.0);
	width[i].set_script("fuse_xsec width", 0);

	max_width_loc[i].initialize(this, UPD_FUSE_XSEC, "Max_Width_Location", 0.0 );
    max_width_loc[i].set_lower_upper(-1000000.0, 1000000.0);
	max_width_loc[i].set_script("fuse_xsec widthoffset", 0);

	corner_rad[i].initialize(this, UPD_FUSE_XSEC, "Corner_Radius", 0.1 );
    corner_rad[i].set_lower_upper(0.0001, 1000000.0);
	corner_rad[i].set_script("fuse_xsec cornerrad", 0);

	top_tan_angle[i].initialize(this, UPD_FUSE_XSEC, "Top_Tan_Angle", 90.0 );
    top_tan_angle[i].set_lower_upper(0.0, 90.0);
	top_tan_angle[i].set_script("fuse_xsec toptanang", 0);

	bot_tan_angle[i].initialize(this, UPD_FUSE_XSEC, "Bot_Tan_Angle", 90.0 );
    bot_tan_angle[i].set_lower_upper(0.0, 90.0);
	bot_tan_angle[i].set_script("fuse_xsec bottanang", 0);

	top_str[i].initialize(this, UPD_FUSE_XSEC, "TopStr", 0.50 );
	top_str[i].set_lower_upper(0.001, 2.0);
	top_str[i].set_script("fuse_xsec alltanstr", 0); // not a bug

	upp_str[i].initialize(this, UPD_FUSE_XSEC, "UppStr", 0.50 );
	upp_str[i].set_lower_upper(0.001, 2.0);
	upp_str[i].set_script("fuse_xsec upcornertanstr", 0);

	low_str[i].initialize(this, UPD_FUSE_XSEC, "LowStr", 0.50 );
	low_str[i].set_lower_upper(0.001, 2.0);
	low_str[i].set_script("fuse_xsec lowcornertanstr", 0);

	bot_str[i].initialize(this, UPD_FUSE_XSEC, "BotStr", 0.50 );
	bot_str[i].set_lower_upper(0.001, 2.0);
	bot_str[i].set_script("fuse_xsec bottanstr", 0);

	allTanStrFlag[i] = 0;
  }

  mlType = IML;    	
  set_type( ELLIPSE );
  mlType = OML;    	
  set_type( ELLIPSE );

  numSectInterp1 = 0;
  numSectInterp2 = 0;

  this->gen_parms();
  this->generate();


}

void fuse_xsec::set_fuse_ptr( Geom* geom_ptr_in )
{
  geom_ptr = geom_ptr_in;

  updateFuse.set_geom(geom_ptr);

  topThick.set_geom( this );
  botThick.set_geom( this );
  sideThick.set_geom( this );
  z_offset.set_geom( this );
  z_offset.set_base_geom( geom_ptr_in );
  location.set_geom( this );

  profileTanStr1.set_geom( this );
  profileTanStr2.set_geom( this );
  profileTanAng.set_geom( this );

  for ( int i = 0 ; i < 2 ; i++ )
  {
	height[i].set_geom( this );
	width[i].set_geom( this );
	max_width_loc[i].set_geom( this );
	corner_rad[i].set_geom( this );
	top_tan_angle[i].set_geom( this );
	bot_tan_angle[i].set_geom( this );
	top_str[i].set_geom( this );
	upp_str[i].set_geom( this );
	low_str[i].set_geom( this );
	bot_str[i].set_geom( this );

	edit_crv[i].set_geom( this );
  }
}

//===== Destructor  =====//
fuse_xsec::~fuse_xsec()
{
}

//===== Copy - Override Equals  =====//
fuse_xsec& fuse_xsec::operator=(const fuse_xsec& ifx) //jrg - look at this...
{
  mlType = ifx.mlType;
  num_pnts = ifx.num_pnts;
//jrg  curr_fuse = ifx.curr_fuse;
//  loc_on_spine = ifx.loc_on_spine;
//  location  = ifx.location;
  z_offset = ifx.z_offset;
  pntSpaceType = ifx.pntSpaceType;

  imlFlag = ifx.imlFlag;
  topThick = ifx.topThick;
  botThick = ifx.botThick;
  sideThick = ifx.sideThick;
  imlXOff = ifx.imlXOff;
  imlZOff = ifx.imlZOff;

  profileTanStr1 = ifx.profileTanStr1;
  profileTanStr2 = ifx.profileTanStr2;
  profileTanAng  = ifx.profileTanAng;

  numSectInterp1 = ifx.numSectInterp1;
  numSectInterp2 = ifx.numSectInterp2;


  for ( int i = 0 ; i < 2 ; i++ )
  {
    xstype[i]   = ifx.xstype[i];
    height[i] = ifx.height[i];
    width[i]  = ifx.width[i];
    max_width_loc[i] = ifx.max_width_loc[i];
    corner_rad[i]    = ifx.corner_rad[i];
    top_tan_angle[i] = ifx.top_tan_angle[i];
    bot_tan_angle[i] = ifx.bot_tan_angle[i];
    top_str[i] = ifx.top_str[i];
    upp_str[i] = ifx.upp_str[i];
    low_str[i] = ifx.low_str[i];
    bot_str[i] = ifx.bot_str[i];
    file_crv[i] = ifx.file_crv[i];
	edit_crv[i] = ifx.edit_crv[i];


	edit_crv[i].set_geom( this );
	edit_crv[i].generate();
  }
  this->gen_parms();
  this->generate();

  return *this;  
}

void fuse_xsec::copy( fuse_xsec & ifx )
{
  mlType = ifx.mlType;
  num_pnts = ifx.num_pnts;

  z_offset = ifx.z_offset.get();
  pntSpaceType = ifx.pntSpaceType;

  imlFlag = ifx.imlFlag;
  topThick = ifx.topThick.get();
  botThick = ifx.botThick.get();
  sideThick = ifx.sideThick.get();
  imlXOff = ifx.imlXOff;
  imlZOff = ifx.imlZOff;

  profileTanStr1 = ifx.profileTanStr1.get();
  profileTanStr2 = ifx.profileTanStr2.get();
  profileTanAng  = ifx.profileTanAng.get();

  numSectInterp1 = ifx.numSectInterp1;
  numSectInterp2 = ifx.numSectInterp2;


  for ( int i = 0 ; i < 2 ; i++ )
  {
    xstype[i]   = ifx.xstype[i];
    height[i] = ifx.height[i].get();
    width[i]  = ifx.width[i].get();
    max_width_loc[i] = ifx.max_width_loc[i].get();
    corner_rad[i]    = ifx.corner_rad[i].get();
    top_tan_angle[i] = ifx.top_tan_angle[i].get();
    bot_tan_angle[i] = ifx.bot_tan_angle[i].get();
    top_str[i] = ifx.top_str[i].get();
    upp_str[i] = ifx.upp_str[i].get();
    low_str[i] = ifx.low_str[i].get();
    bot_str[i] = ifx.bot_str[i].get();
    file_crv[i] = ifx.file_crv[i];
	edit_crv[i] = ifx.edit_crv[i];

	edit_crv[i].set_geom( this );
	edit_crv[i].generate();
  }
  this->gen_parms();
  this->generate();


}


void fuse_xsec::setAllTanStrFlag( int f )		
{ 
	allTanStrFlag[mlType] = f; 

	if (f)
		top_str[mlType].set_script("fuse_xsec alltanstr", 0);
	else
		top_str[mlType].set_script("fuse_xsec toptanstr", 0);

}


//==== Draw Cross Section In 2D Window ====//
void fuse_xsec::draw()
{
	int i;

	glPushMatrix();

	glScalef( (float)drawScaleFactor, (float)drawScaleFactor, 1.0 );

	//==== Draw Grid ====//
	float gridSize;
	if ( drawScaleFactor < 0.025 )		gridSize = 10.0f;
	else if ( drawScaleFactor < 0.25 )	gridSize = 1.0f;
	else								gridSize = 0.1f;

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

/*
	glLineWidth(1.0);

	//==== Draw Cross ====//
	glColor3f(0.8f, 0.8f, 0.8f);

	glBegin( GL_LINES );
	glVertex2f(  0.0f, -1.0f );
	glVertex2f(  0.0f,  1.0f );
	glVertex2f( -1.0f,  0.0f );
	glVertex2f(  1.0f,  0.0f );
	glEnd();
*/
	//==== Find Zoom Value ====//
//	double scale = drawScaleFactor;
//	double max = MAX( width[mlType](), height[mlType]() );

//	if ( max > 0.0001 )
//		scale = drawScaleFactor*(1.0/max);

//	glPushMatrix();
//	glScalef( scale, scale, 1.0 );

	glColor3f(1.0, 0.0, 0.0);
	glBegin( GL_LINE_STRIP );
    for (  i = 0 ; i < num_pnts ; i++)
    {
		vec3d p = pnts[0][i];
		glVertex2d( p.y(), p.z() );
	}
	glEnd();

 	if ( imlFlag )
	{
		double z = this->get_iml_z_offset();

		glBegin( GL_LINE_STRIP );
		for (  int i = 0 ; i < num_pnts ; i++)
		{
			vec3d p = pnts[1][i];
			glVertex2d( p.y(), p.z() + z );
		}
		glEnd();
	}

	glColor3f(0.0, 0.0, 1.0);
	glBegin( GL_LINES );
		
	vec3d p = top_crv[mlType].get_pnt(0);
	glVertex2d( p.y(), p.z() );
	p = p + top_crv[mlType].get_tan(0);
	glVertex2d( p.y(), p.z() );

	p = top_crv[mlType].get_pnt(1);
	glVertex2d( p.y(), p.z() );
	p = p - top_crv[mlType].get_tan(1);
	glVertex2d( p.y(), p.z() );
	
	p = bot_crv[mlType].get_pnt(0);
	glVertex2d( p.y(), p.z() );
	p = p + bot_crv[mlType].get_tan(0);
	glVertex2d( p.y(), p.z() );

	p = bot_crv[mlType].get_pnt(1);
	glVertex2d( p.y(), p.z() );
	p = p - bot_crv[mlType].get_tan(1);
	glVertex2d( p.y(), p.z() );

	glEnd();
 

	glPopMatrix();

}

//==== Parm Has Changed ReGenerate Component ====//
void fuse_xsec::parm_changed(Parm* p)
{
	int up_group = p->get_update_grp();

	switch (up_group)
    {
		case UPD_FUSE_XSEC:
		{
			regenerate();

			//==== Trigger Fuse Geom Update ====//
			if ( geom_ptr )
				geom_ptr->parm_changed( &updateFuse );

		}
		case UPD_EDIT_CRV:
		{
			regenerate();
/*
			for ( int i = 0 ; i < 2 ; i++ )
			{
				width[i]  = edit_crv[i].getMaxWidth()->get();
				height[i] = edit_crv[i].getMaxHeight()->get();
			}
*/

//		edit_crv[mlType].scaleWH( width[mlType](), height[mlType]() );


			//==== Trigger Fuse Geom Update ====//
			if ( geom_ptr )
				geom_ptr->parm_changed( &updateFuse );
		}
	}


}


//==== Set Type Of Cross Section  ====//
void fuse_xsec::set_type(int type_in)           
{ 
 	xstype[mlType] = type_in; 

	//==== Deactivate Everything ====//
	height[mlType].deactivate();
	width[mlType].deactivate();
	max_width_loc[mlType].deactivate();
	corner_rad[mlType].deactivate();
	top_tan_angle[mlType].deactivate();
	bot_tan_angle[mlType].deactivate();
	top_str[mlType].deactivate();
	upp_str[mlType].deactivate();
	low_str[mlType].deactivate();
	bot_str[mlType].deactivate();

	if ( xstype[mlType] == XSEC_POINT )
	{
	}
	else if ( xstype[mlType] == CIRCLE )
	{
		height[mlType].activate();
	}
	else if ( xstype[mlType] == ELLIPSE )
	{
		height[mlType].activate();
		width[mlType].activate();
	}
	else if ( xstype[mlType] == RND_BOX )
	{
		height[mlType].activate();
		width[mlType].activate();
		top_str[mlType].activate();
	}
	else if ( xstype[mlType] == GENERAL )
	{
		height[mlType].activate();
		width[mlType].activate();
		max_width_loc[mlType].activate();
		corner_rad[mlType].activate();
		top_tan_angle[mlType].activate();
		bot_tan_angle[mlType].activate();
		top_str[mlType].activate();
		upp_str[mlType].activate();
		low_str[mlType].activate();
		bot_str[mlType].activate();
	}
	else if ( xstype[mlType] == FROM_FILE )
	{
		height[mlType].activate();
		width[mlType].activate();
	}
	else if ( xstype[mlType] == EDIT_CRV )
	{
//		edit_crv[mlType].scaleWH( width[mlType](), height[mlType]() );
	}

}

void fuse_xsec::setDrawScaleFactor( double sf )
{
	rollerScaleFactor = sf;
	drawScaleFactor = (double)pow(10.0, rollerScaleFactor)/1000.0;
}

//==== Set Point On Span Offset by ZOffset  ====//
void fuse_xsec::set_pnt_on_spine(const vec3d& offset)
{
  pnt_on_spine = offset;
  pnt_on_spine.offset_z(z_offset() );

  iml_pnt_on_spine = offset;
  iml_pnt_on_spine.offset_x( imlXOff );
  iml_pnt_on_spine.offset_z( z_offset() + imlZOff );
}

//==== Get Point On Span Offset by ZOffset and IML ZOffset ====//
vec3d& fuse_xsec::get_iml_pnt_on_spine()
{
  return iml_pnt_on_spine;
}

//==== Set Mold Line Type ====//
void fuse_xsec::set_ml_type( int ml_type_in )
{
  assert( ml_type_in == IML || ml_type_in == OML );
  mlType = ml_type_in;

}

void fuse_xsec::setEditCrv( EditCurve* crv )
{ 
	edit_crv[mlType] = *crv; 
	edit_crv[mlType].set_geom( geom_ptr );
//	edit_crv[mlType].scaleWH( width[mlType](), height[mlType]() );
}



//==== Get Mold Line Type ====//
int fuse_xsec::get_ml_type( )
{
  return mlType;
}

void fuse_xsec::set_iml_flag( int onoff )
{
  imlFlag = onoff;
  if ( imlFlag == 0 )
  {
    set_ml_type( OML );
  }
}


//==== Thickness Change - Modify Non-Driver ====//
void fuse_xsec::thickChange()
{
  if ( !imlFlag  )
    return;

  if ( mlType == IML )
  {
    if ( xstype[IML] == CIRCLE )
    {
      sideThick = 0.00001f;
    }

    double ht = height[OML]() - (topThick() + botThick());
    if ( ht <= 0.00001 )
      ht = 0.00001;

    double wd = width[OML]() - 2.0*sideThick();
    if ( wd < 0.00001 )
      wd = 0.00001;

    height[IML]  = ht;
    width[IML]   = wd;

    if ( xstype[IML] == CIRCLE )
    {
      width[IML] = height[IML]();
    }
  }
  else if ( mlType == OML )
  {
    if ( xstype[OML] == CIRCLE )
    {
      sideThick = 0.00001;
    }

    double ht = height[IML]() + (topThick() + botThick());
    if ( ht <= 0.00001 )
      ht = 0.00001;

    double wd = width[IML]() + 2.0*sideThick();
    if ( wd < 0.00001 )
      wd = 0.00001;

    height[OML]  = ht;
    width[OML]   = wd;

    if ( xstype[OML] == CIRCLE )
    {
      width[OML] = height[OML]();
    }

  }
//  curr_fuse->comp_spine();

}

//==== Set NUmber of Points ====//
void fuse_xsec::set_num_pnts(int num_pnts_in)
{
	if ( num_pnts_in > 8 )
	{
		if ( num_pnts_in%2 == 0 )
			num_pnts_in++;
		num_pnts = num_pnts_in;

		pnts[0].init(num_pnts);
		pnts[1].init(num_pnts);
		this->gen_parms();
		this->generate();
	}
}


//==== Write Fuse Cross Section File ====//
void fuse_xsec::write(FILE* file_id)
{
  write_parms( file_id, OML );

  if ( imlFlag )
  {
    write_parms( file_id, IML );
  }

}

//==== Write Fuse Cross Section File ====//
void fuse_xsec::write(xmlNodePtr root)
{

  xmlAddIntNode( root, "Num_Pnts", num_pnts);
  xmlAddDoubleNode( root, "Spine_Location", location() );
  xmlAddDoubleNode( root, "Z_Offset", z_offset());

  xmlAddDoubleNode( root, "Top_Thick", topThick());
  xmlAddDoubleNode( root, "Bot_Thick", botThick());
  xmlAddDoubleNode( root, "Side_Thick", sideThick());

  xmlAddDoubleNode( root, "Act_Top_Thick", actTopThick);
  xmlAddDoubleNode( root, "Act_Bot_Thick", actBotThick);
  xmlAddDoubleNode( root, "Act_Side_Thick", actSideThick);

  xmlAddDoubleNode( root, "IML_X_Offset", imlXOff);
  xmlAddDoubleNode( root, "IML_Z_Offset", imlZOff);

  xmlAddIntNode( root, "ML_Type",  mlType);
  xmlAddIntNode( root, "IML_Flag", imlFlag);

  //==== Profile Stuff ====//
  xmlAddDoubleNode( root, "Profile_Tan_Str_1", profileTanStr1());
  xmlAddDoubleNode( root, "Profile_Tan_Str_2", profileTanStr2());
  xmlAddDoubleNode( root, "Profile_Tan_Ang", profileTanAng());

  xmlAddIntNode( root, "Num_Sect_Interp_1", numSectInterp1);
  xmlAddIntNode( root, "Num_Sect_Interp_2", numSectInterp2);

  //==== Outer Mold Line Parms ====//
  xmlNodePtr oml_node = xmlNewChild( root, NULL, (const xmlChar *)"OML_Parms", NULL );
  write_parms( oml_node, OML );

  //==== Inner Mold Line Parms ====//
  xmlNodePtr iml_node = xmlNewChild( root, NULL, (const xmlChar *)"IML_Parms", NULL );
  write_parms( iml_node, IML );

}


//==== Write Fuse Cross Section File ====//
void fuse_xsec::write_parms(FILE* file_id, int mltype )
{
  fprintf(file_id, "%d				Fuse Xsec Type\n",xstype[mltype]);


  if ( mltype == OML )
  {
    fprintf(file_id, "%f			Z_Offset\n",z_offset() );
    fprintf(file_id, "%f			Location On Spine\n",location() );
    fprintf(file_id, "%d			IML_Flag\n", imlFlag );
    fprintf(file_id, "%d			Number of Pnts Per Xsec\n",num_pnts);
  }
  else if ( mltype == IML )
  {
    fprintf(file_id, "%d			ML_Type\n", mlType );
    fprintf(file_id, "%f			Top Thick\n",topThick());
    fprintf(file_id, "%f			Bot Thick\n",botThick());
    fprintf(file_id, "%f			Side Thick\n",sideThick());
    fprintf(file_id, "%f			Act Top Thick\n",actTopThick);
    fprintf(file_id, "%f			Act Bot Thick\n",actBotThick);
    fprintf(file_id, "%f			Act Side Thick\n",actSideThick);
    fprintf(file_id, "%f			IML X Offset\n",imlXOff);
    fprintf(file_id, "%f			IML Z Offset\n",imlZOff);

  }

  if ( xstype[mltype] == XSEC_POINT )
    return;

  fprintf(file_id, "%f			Height\n",height[mltype]() );

  if ( xstype[mltype] == CIRCLE )
    return;

  fprintf(file_id, "%f			Width\n",width[mltype]() );

  if ( xstype[mltype] == ELLIPSE )
    return;

  if ( xstype[mltype] == FROM_FILE )
  {
    vec3d pnt;
    fprintf( file_id, "%s			File Name\n", fileName() );
    fprintf( file_id, "%d			Num Pnts\n",  file_crv[mltype].get_num_pnts() );

    for ( int i = 0 ; i < file_crv[mltype].get_num_pnts() ; i++ )
    {
      pnt =  file_crv[mltype].get_pnt( i );
      fprintf( file_id, "%f  %f			Y, Z Pnt \n",  pnt.y(), pnt.z() );
    }
    return;
  }

  fprintf(file_id, "%f			Top Tan Strength\n",top_str[mltype]() );
  fprintf(file_id, "%f			Upper Tan Strength\n",upp_str[mltype]() );
  fprintf(file_id, "%f			Lower Tan Strength\n",low_str[mltype]() );
  fprintf(file_id, "%f			Bottom Tan Strength\n",bot_str[mltype]() );

  if ( xstype[mltype] == RND_BOX )
    return;
  
  fprintf(file_id, "%f			Max Width Location\n",max_width_loc[mltype]() );
  fprintf(file_id, "%f			Corner Radius\n",corner_rad[mltype]() );
  fprintf(file_id, "%f			Top Tan Angle (rad)\n",top_tan_angle[mltype]() );
  fprintf(file_id, "%f			Bot Tan Angle (rad)\n",bot_tan_angle[mltype]() );
}

//==== Write Fuse Cross Section File ====//
void fuse_xsec::write_parms(xmlNodePtr root, int mltype )
{
  xmlAddIntNode( root, "Type", xstype[mltype]);

  xmlAddDoubleNode( root, "Height", height[mltype]() );
  xmlAddDoubleNode( root, "Width",  width[mltype]() );

  xmlAddDoubleNode( root, "Max_Width_Location", max_width_loc[mltype]() );
  xmlAddDoubleNode( root, "Corner_Radius", corner_rad[mltype]() );
  xmlAddDoubleNode( root, "Top_Tan_Angle", top_tan_angle[mltype]() );
  xmlAddDoubleNode( root, "Bot_Tan_Angle", bot_tan_angle[mltype]() );

  xmlAddDoubleNode( root, "Top_Tan_Strength",    top_str[mltype]() );
  xmlAddDoubleNode( root, "Upper_Tan_Strength",  upp_str[mltype]() );
  xmlAddDoubleNode( root, "Lower_Tan_Strength",  low_str[mltype]() );
  xmlAddDoubleNode( root, "Bottom_Tan_Strength", bot_str[mltype]() );

  if ( xstype[mltype] == FROM_FILE )
  {
    vec3d pnt;
    Stringc ystr, zstr;
    char numstr[255];
    int num_pnts = file_crv[mltype].get_num_pnts();

    xmlAddStringNode( root, "File_Name", fileName );
    for ( int i = 0 ; i < file_crv[mltype].get_num_pnts() ; i++ )
    {
      pnt = file_crv[mltype].get_pnt( i );
      sprintf( numstr, "%lf,", pnt.y() );
      ystr.concatenate(numstr);
      sprintf( numstr, "%lf,", pnt.z() );
      zstr.concatenate(numstr);
    }
    ystr.concatenate("\0");
    zstr.concatenate("\0");

    xmlAddStringNode( root, "File_Y_Pnts", ystr );
    xmlAddStringNode( root, "File_Z_Pnts", zstr );
  }
  else if ( xstype[mltype] == EDIT_CRV )
  {
	edit_crv[mlType].write( root );
  }
}

//==== Read Fuse Cross Section File ====//
void fuse_xsec::read(xmlNodePtr node)
{
  num_pnts = xmlFindInt( node, "Num_Pnts", num_pnts );
  location = xmlFindDouble( node, "Spine_Location", location() );
  z_offset =  xmlFindDouble( node, "Z_Offset", z_offset() );

  topThick  = xmlFindDouble( node, "Top_Thick", topThick() );
  botThick  = xmlFindDouble( node, "Bot_Thick", botThick() );
  sideThick = xmlFindDouble( node, "Side_Thick", sideThick() );

  actTopThick   = xmlFindDouble( node, "Act_Top_Thick", actTopThick );
  actBotThick   = xmlFindDouble( node, "Act_Bot_Thick", actBotThick );
  actSideThick  = xmlFindDouble( node, "Act_Side_Thick", actSideThick );

  imlXOff = xmlFindDouble( node, "IML_X_Offset", imlXOff );
  imlZOff = xmlFindDouble( node, "IML_Z_Offset", imlZOff );

  imlFlag = xmlFindInt( node, "IML_Flag", imlFlag );
  mlType  = xmlFindInt( node, "ML_Type", mlType );

  //==== Profile Stuff ====//
  profileTanStr1   = xmlFindDouble( node, "Profile_Tan_Str_1", profileTanStr1() );
  profileTanStr2   = xmlFindDouble( node, "Profile_Tan_Str_2", profileTanStr2() );
  profileTanAng    = xmlFindDouble( node, "Profile_Tan_Ang",   profileTanAng() );

  numSectInterp1  = xmlFindInt( node, "Num_Sect_Interp_1", numSectInterp1 );
  numSectInterp2  = xmlFindInt( node, "Num_Sect_Interp_2", numSectInterp2 );

  xmlNodePtr omlNode = xmlGetNode( node, "OML_Parms", 0 );
  if ( omlNode )
    read_parms( omlNode, OML );

  if ( imlFlag )
  {
    xmlNodePtr imlNode = xmlGetNode( node, "IML_Parms", 0 );
    read_parms( imlNode, IML );
  }
}

//==== Read Fuse Cross Section File ====//
void fuse_xsec::read_parms(xmlNodePtr node, int mltype)
{
  int i;
  xstype[mltype] = xmlFindInt( node, "Type", xstype[mltype] );

  height[mltype] =  xmlFindDouble( node, "Height", height[mltype]() );
  width[mltype]  =  xmlFindDouble( node, "Width",  width[mltype]() );

  max_width_loc[mltype]  = xmlFindDouble( node, "Max_Width_Location",  max_width_loc[mltype]() );
  corner_rad[mltype]     = xmlFindDouble( node, "Corner_Radius",  corner_rad[mltype]() );
  top_tan_angle[mltype] = xmlFindDouble( node, "Top_Tan_Angle", top_tan_angle[mltype]());
  bot_tan_angle[mltype] = xmlFindDouble( node, "Bot_Tan_Angle", bot_tan_angle[mltype]());

  top_str[mltype]  = xmlFindDouble( node, "Top_Tan_Strength",    top_str[mltype]() );
  upp_str[mltype]  = xmlFindDouble( node, "Upper_Tan_Strength",  upp_str[mltype]() );
  low_str[mltype]  = xmlFindDouble( node, "Lower_Tan_Strength",  low_str[mltype]() );
  bot_str[mltype]  = xmlFindDouble( node, "Bottom_Tan_Strength", bot_str[mltype]() );

  if ( xstype[mltype] == FROM_FILE )
  {
    xstype[mltype] = GENERAL;				// If this fails fall back on something...

	fileName = Stringc( xmlFindString( node, "File_Name", "File_Xsec" ) );					// fileName is a Stringc

    xmlNodePtr yn = xmlGetNode( node, "File_Y_Pnts", 0 );
    xmlNodePtr zn = xmlGetNode( node, "File_Z_Pnts", 0 );

    if ( yn && zn )
    {
      int numy = xmlGetNumArray( yn, ',' );
      int numz = xmlGetNumArray( zn, ',' );
      if ( numy == numz )
      {
        int num_pnts = numy;
        double* arry = (double*)malloc( num_pnts*sizeof(double) );
        double* arrz = (double*)malloc( num_pnts*sizeof(double) );
        xmlExtractDoubleArray( yn, ',', arry, num_pnts );
        xmlExtractDoubleArray( zn, ',', arrz, num_pnts );

        xstype[mltype] = FROM_FILE;	

        file_crv[mltype].init( num_pnts );

        for ( i = 0 ; i < num_pnts ; i++ )
        {
          file_crv[mltype].load_pnt( i, vec3d( 0.0, arry[i],  arrz[i] ));
        }
        free( arry );
        free( arrz );

        file_crv[mltype].comp_tans();

        vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
        double tan_mag = file_crv[mltype].get_tan(0).mag();

        file_crv[mltype].load_tan( 0, top_tan*tan_mag );

        int last_id =  file_crv[mltype].get_num_pnts() - 1;
        vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
        tan_mag = file_crv[mltype].get_tan( last_id ).mag();
        file_crv[mltype].load_tan( last_id, bot_tan*tan_mag );
      }
    }
  }
  else if ( xstype[mltype] == EDIT_CRV )
  {
	edit_crv[mltype].read( node );
  }
  set_type(xstype[mltype]);  

}

//==== Read Fuse Cross Section File ====//
void fuse_xsec::read(FILE* file_id)
{
  read_parms( file_id, OML );

  if ( imlFlag )
    read_parms( file_id, IML );
}

//==== Read Fuse Cross Section File ====//
void fuse_xsec::read_parms(FILE* file_id, int mltype)
{
//jrg check this - read parm wrong???
  int i;
  char buff[255];
  fscanf(file_id, "%d",&xstype[mltype]);		fgets(buff, 80, file_id);
  set_type(xstype[mltype]);  

  //===== Hack to Check For Z_Offset =====//
  if ( mltype == OML )
  {
    float fdum;
    fscanf(file_id, "%f",&fdum);	fgets(buff, 80, file_id);
    Stringc parm_desc(buff);
    if ( parm_desc.search_for_substring("Location") >= 0)
      {
        location.set(fdum);
      }
    else
      {
        z_offset.set(fdum);
        fscanf(file_id, "%f",&fdum);	fgets(buff, 80, file_id);
		location.set(fdum);
      }
//printf("2buff = %s \n", buff );

    //===== Hack to Check For IML Flag =====//
    int idum;
    fscanf(file_id, "%d",&idum);	fgets(buff, 80, file_id);
    Stringc strc(buff);
    if ( strc.search_for_substring("IML_Flag") >= 0)
    {
      imlFlag = idum;
      fscanf(file_id, "%d",&num_pnts);	fgets(buff, 80, file_id);
    }
    else
    {
      num_pnts = idum;
    }
  }
  else if ( mltype == IML )
  {
	  float tt, bt, st, xoff, yoff;
    fscanf(file_id, "%d",&mlType);	      fgets(buff, 80, file_id);
	topThick.read(file_id);		//fscanf(file_id, "%f",&topThick);	    fgets(buff, 80, file_id);
    botThick.read(file_id);		//fscanf(file_id, "%f",&botThick);	    fgets(buff, 80, file_id);
    sideThick.read(file_id);		//fscanf(file_id, "%f",&sideThick);	    fgets(buff, 80, file_id);
    fscanf(file_id, "%f",&tt);	  fgets(buff, 80, file_id);
    fscanf(file_id, "%f",&bt);	  fgets(buff, 80, file_id);
    fscanf(file_id, "%f",&st);  fgets(buff, 80, file_id);
    fscanf(file_id, "%f",&xoff);	      fgets(buff, 80, file_id);
    fscanf(file_id, "%f",&yoff);	      fgets(buff, 80, file_id);
	actTopThick = tt;
	actBotThick = bt;
	actSideThick = st;
	imlXOff = xoff;
	imlZOff = yoff;





  }

  if ( xstype[mltype] == XSEC_POINT )
    return;

  height[mltype].read( file_id );	
//  double val_read_in;
//  fscanf(file_id, "%f",&val_read_in);	fgets(buff, 80, file_id);
//  height[mltype] = val_read_in;

  if ( xstype[mltype] == CIRCLE )
    return;

  width[mltype].read( file_id );	
//  fscanf(file_id, "%f",&val_read_in);		fgets(buff, 80, file_id);
//  width[mltype] = val_read_in;

  if ( xstype[mltype] == ELLIPSE )
    return;

  if ( xstype[mltype] == FROM_FILE )
  {
    int num_pnts;
    float y, z;
    char str[255];

    fscanf(file_id, "%s", str);	fgets(buff, 80, file_id);
    fileName = str;
    fscanf(file_id, "%d", &num_pnts);	fgets(buff, 80, file_id);
    file_crv[mltype].init( num_pnts );

    for ( i = 0 ; i < num_pnts ; i++ )
    {
      fscanf(file_id, "%f  %f", &y, &z);  fgets(buff, 80, file_id);
      file_crv[mltype].load_pnt( i, vec3d( 0.0, y,  z ));
    }

    file_crv[mltype].comp_tans();

    vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
    double tan_mag = file_crv[mltype].get_tan(0).mag();

    file_crv[mltype].load_tan( 0, top_tan*tan_mag );

    int last_id =  file_crv[mltype].get_num_pnts() - 1;
    vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
    tan_mag = file_crv[mltype].get_tan( last_id ).mag();
    file_crv[mltype].load_tan( last_id, bot_tan*tan_mag );

    return;
  }

  top_str[mltype].read( file_id );  //fscanf(file_id, "%f",&top_str[mltype]);	fgets(buff, 80, file_id);
  upp_str[mltype].read( file_id );  //fscanf(file_id, "%f",&upp_str[mltype]);	fgets(buff, 80, file_id);
  low_str[mltype].read( file_id );  //fscanf(file_id, "%f",&low_str[mltype]);	fgets(buff, 80, file_id);
  bot_str[mltype].read( file_id );  //fscanf(file_id, "%f",&bot_str[mltype]);	fgets(buff, 80, file_id);

  if ( xstype[mltype] == RND_BOX )
    return;

  max_width_loc[mltype].read(file_id ); // fscanf(file_id, "%f",&max_width_loc[mltype]);	fgets(buff, 80, file_id);
  corner_rad[mltype].read(file_id ); //  fscanf(file_id, "%f",&corner_rad[mltype]);	fgets(buff, 80, file_id);

  float angle_read_in;
  fscanf(file_id, "%f",&angle_read_in); fgets(buff, 80, file_id);
  top_tan_angle[mltype] = angle_read_in*RAD_2_DEG; 

  fscanf(file_id, "%f",&angle_read_in);	        fgets(buff, 80, file_id);
  bot_tan_angle[mltype] = angle_read_in*RAD_2_DEG;

}


//===== Regenerate Current Fuse Cross Section And Fuse  =====//
void fuse_xsec::regenerate()
{
  gen_parms();
  generate();
//  curr_fuse->xsec_trigger();
}


//===== Generate All Parmeters For X-Sec  =====//
void fuse_xsec::gen_parms()
{
  //==== For Both OML & IML ====//
  for ( int i = 0 ; i < 2 ; i++ )
  {
   switch ( xstype[i] )
    {
      case XSEC_POINT:
        height[i] = 0.0;
        width[i]  = 0.0;
        max_width_loc[i] = 0.0;
        corner_rad[i] = 0.0;
        top_tan_angle[i] = 90.0;
        bot_tan_angle[i] = 90.0;
        top_str[i] = upp_str[i] = low_str[i] = bot_str[i] = 0.83;
      break;

      case CIRCLE:
        width[i]  = height[i]();
        max_width_loc[i] = 0.0;
        corner_rad[i] = 0.5;
        top_tan_angle[i] = 90.0;
        bot_tan_angle[i] = 90.0;
        top_str[i] = upp_str[i] = low_str[i] = bot_str[i] = .83;
      break;

      case ELLIPSE:
        max_width_loc[i] = 0.0;
        corner_rad[i] = 0.5;
        top_tan_angle[i] = 90.0;
        bot_tan_angle[i] = 90.0;
        top_str[i] = upp_str[i] = low_str[i] = bot_str[i] = .83;
      break;

      case FROM_FILE:
        max_width_loc[i] = 0.0;
        corner_rad[i] = 0.5;
        top_tan_angle[i] = 90.0;
        bot_tan_angle[i] = 90.0;
        top_str[i] = upp_str[i] = low_str[i] = bot_str[i] = .83;
      break;

      case RND_BOX:
        max_width_loc[i] = 0.0;
        top_tan_angle[i] = 90.0;
        bot_tan_angle[i] = 90.0;
        corner_rad[i] = 0.5;
      break;

	  case EDIT_CRV:
//		  width[i] = 2.2;
//		  double junk = edit_crv[i].getMaxWidth()->get();
//		width[i]  = edit_crv[i].getMaxWidth()->get();
//		height[i] = edit_crv[i].getMaxHeight()->get();
	  break;

   }
  }

}


//===== Generate  Xsec  =====//
void fuse_xsec::generate()
{
//  debug_print();

  //==== Account for ML if Needed ====//
  if ( imlFlag && !interpThickFlag)
  {
    if ( mlType == OML )
    {
      double ht = height[OML]() - (topThick() + botThick());
      if ( ht <= 0.00001 )
        ht = 0.00001;

      double wd = width[OML]() - 2.0*sideThick();
      if ( wd < 0.00001 )
        wd = 0.00001;

      double currHt = height[IML]();
      double currWd = width[IML]();

      if ( xstype[OML] == XSEC_POINT )
        xstype[IML] = XSEC_POINT;

      if ( xstype[IML] == XSEC_POINT )
			{
        height[IML] = width[IML] = 0.0;
			}
      else
			{
        double wScale = 1.0;
        if ( currWd > 0.000001 )
					wScale = wd/currWd;
        double hScale = 1.0;
        if ( currHt > 0.000001 )
					hScale = ht/currHt;

        double scale = wScale;
        if ( hScale < wScale )
          scale = hScale;

        height[IML] = currHt*scale;
        width[IML]  = currWd*scale;

        double topRatio = 0.5;
        double tb = topThick() + botThick();
        if ( tb > 0.000001 ) topRatio = topThick()/tb;
        double botRatio = 1.0 - topRatio;

        actSideThick =  0.5*(width[OML]() - width[IML]());
        actTopThick  =  topRatio*(height[OML]() - height[IML]());
        actBotThick  =  botRatio*(height[OML]() - height[IML]());

        imlZOff =  height[OML]()/2.0 - height[IML]()/2.0 - actTopThick;
			}
    }
    else if ( mlType == IML )
    {
      double ht = height[IML]() + (topThick() + botThick() );
      if ( ht <= 0.00001 )
        ht = 0.00001;

      double wd = width[IML]() + 2.0*sideThick();
      if ( wd < 0.00001 )
        wd = 0.00001;

      double currHt = height[OML]();
      double currWd = width[OML]();

      if ( xstype[IML] == XSEC_POINT )
        xstype[OML] = XSEC_POINT;

      if ( xstype[OML] == XSEC_POINT )
	  {
        height[OML] = width[OML] = 0.0;
	  }
      else
	  {
        double wScale = 1.0;
        if ( currWd > 0.000001 )
					wScale = wd/currWd;
        double hScale = 1.0;
        if ( currHt > 0.000001 )
					hScale = ht/currHt;

        double scale = wScale;
        if ( hScale > wScale )
          scale = hScale;

        height[OML] = currHt*scale;
        width[OML]  = currWd*scale;

        double topRatio = 0.5;
        double tb = topThick() + botThick();
        if ( tb > 0.000001 ) topRatio = topThick()/tb;
        double botRatio = 1.0 - topRatio;

        actSideThick =  0.5*(width[OML]() - width[IML]());
        actTopThick  =  topRatio*(height[OML]() - height[IML]());
        actBotThick  =  botRatio*(height[OML]() - height[IML]());

        imlZOff =  height[OML]()/2.0 - height[IML]()/2.0 - actTopThick;
	  }
    }
  }


  //==== Do IML & OML ====//
  for ( int i = 0 ; i < 2 ; i++ )
  {
   if ( xstype[i] == FROM_FILE )
   {
     pnts[i].init(num_pnts);
     load_file_pnts( i );
   }
   else if ( xstype[i] == EDIT_CRV )
   {
     pnts[i].init(num_pnts);
     load_edit_crv_pnts( i );
   }

   else if ( i == OML || (i == IML && imlFlag  ) )
   {
     double y, z;

	 if ( xstype[i] != GENERAL )
	 {
		 upp_str[i] = low_str[i] = bot_str[i] =  top_str[i]();
	 }

	 //jrg - 8/4/09
 	 if ( allTanStrFlag[i] )
	 {
		 upp_str[i] = low_str[i] = bot_str[i] =  top_str[i]();
	 }


     y = 0.0;
     z = height[i]()/2.0;
     top_crv[i].load_pnt( 0, vec3d( 0.0, y, z ));

     y = (double)(top_str[i]()*width[i]());
     z = 0.0;
     top_crv[i].load_tan( 0, vec3d( 0.0, y, z ));

     y = (double)( width[i]()/2.0 + corner_rad[i]()*height[i]()*(sin(DEG_2_RAD*top_tan_angle[i]())-1.0) );
     z = (double)(max_width_loc[i]()*height[i]()/2.0 + corner_rad[i]()*height[i]()*cos(DEG_2_RAD*top_tan_angle[i]()) ) ;
     top_crv[i].load_pnt( 1, vec3d( 0.0, y, z ));

     y = (double)( upp_str[i]()*height[i]()*cos(DEG_2_RAD*top_tan_angle[i]()) );
     z = (double)( -upp_str[i]()*height[i]()*sin(DEG_2_RAD*top_tan_angle[i]()) );
     top_crv[i].load_tan( 1, vec3d( 0.0, y, z ));

     y = (double)(width[i]()/2.0 + corner_rad[i]()*height[i]()*(sin(DEG_2_RAD*bot_tan_angle[i]())-1.0) );
     z = (double)(max_width_loc[i]()*height[i]()/2.0 - corner_rad[i]()*height[i]()*cos(DEG_2_RAD*bot_tan_angle[i]()) );
     bot_crv[i].load_pnt( 0, vec3d( 0.0, y, z ));

     y = (double)( -low_str[i]()*height[i]()*cos(DEG_2_RAD*bot_tan_angle[i]()) );
     z = (double)( -low_str[i]()*height[i]()*sin(DEG_2_RAD*bot_tan_angle[i]()) );
     bot_crv[i].load_tan( 0, vec3d( 0.0, y, z ));

     y = 0.0;
     z = (double)( -height[i]()/2.0 );
     bot_crv[i].load_pnt( 1, vec3d( 0.0, y, z ));

     y = (double)( -bot_str[i]()*width[i]() );
     z = 0.0;
     bot_crv[i].load_tan( 1, vec3d( 0.0, y, z ));

     pnts[i].init(num_pnts);
     if ( pntSpaceType == PNT_SPACE_FIXED || pntSpaceType == PNT_SPACE_PER_XSEC )
     {
       load_pnts(i);
     }
     else if ( pntSpaceType == PNT_SPACE_UNIFORM )
     {
       load_uniform_pnts(i);
     }
   }
  }
}

//===== Load Points from File Curve ====//
void fuse_xsec::load_file_pnts( int mlt )
{
   int j;
   double u;
   double zfact = height[mlt]()/2.0;
   double yfact = width[mlt]()/2.0;

   int num_pnt_half = num_pnts/2 + 1;

   int cnt = 0;

   //==== Top Curve Points ====//
   for ( j = 0 ; j < num_pnt_half ; j++ )
     {
       u = (double)j/(double)(num_pnt_half-1);
       pnts[mlt][cnt] = file_crv[mlt].comp_pnt_per_length((float)u);
       pnts[mlt][cnt].scale_y( yfact );
       pnts[mlt][cnt].scale_z( zfact );
       cnt++;
     }

  //==== Reflection Points ====//
  int num_half_pnts = cnt;
  for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
    {
      pnts[mlt][cnt] = pnts[mlt][j].reflect_xz();
      cnt++;
    }

}

void fuse_xsec::load_edit_crv_pnts( int mlt )
{
	vec3d p;

	Bezier_curve crv = edit_crv[mlt].getBezierCurve();

	for ( int i = 0 ; i < num_pnts ; i++ )
	{
		float u = (float)i/(float)(num_pnts-1);
		p = crv.comp_pnt(u);
        pnts[mlt][i] = vec3d( 0.0, p.x(), p.y() );
	}

}


//===== Read AF File =====//
int fuse_xsec::read_xsec_file( Stringc file_name )
{
  int i;
  Stringc line;
  char buff[255];

  /* --- Open file --- */
  FILE* file_id = fopen(file_name, "r");

  if (file_id == (FILE *)NULL) return(0);

  fgets(buff, 80, file_id);  line = buff;

  if (line.search_for_substring("FUSE XSEC FILE") < 0)
    return 0;

  char name[255];
  int num_pnts;
  float y, z;

  fscanf(file_id, "%s", name);       fgets(buff, 80, file_id);
  fileName = name;
  fscanf(file_id, "%d", &num_pnts);  fgets(buff, 80, file_id);

  file_crv[mlType].init( num_pnts );

  for ( i = 0 ; i < num_pnts ; i++ )
  {
    fscanf(file_id, "%f  %f", &y, &z);  fgets(buff, 80, file_id);

    file_crv[mlType].load_pnt( i, vec3d( 0.0, y,  z ));
  }

  //==== Find Height & Width ====//
  vec3d p0 = file_crv[mlType].get_pnt( 0 );
  vec3d pn = file_crv[mlType].get_pnt( num_pnts-1 );
  height[mlType] = fabs(p0.z() - pn.z());

  double max_y = 0;
  for ( i = 0 ; i < num_pnts ; i++ )
  {
    double y = file_crv[mlType].get_pnt( i ).y();
    if  ( fabs(y) > max_y )
      max_y = fabs(y);
  }
  width[mlType] = (2.0*max_y);

  //==== Scale Point By Height & Width ====//
  for ( i = 0 ; i < num_pnts ; i++ )
  {
    double y = 2.0*file_crv[mlType].get_pnt( i ).y()/width[mlType]();
    double z = 2.0*file_crv[mlType].get_pnt( i ).z()/height[mlType]();
    file_crv[mlType].load_pnt( i, vec3d( 0.0, y,  z ));

  }


  file_crv[mlType].comp_tans();

  vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
  double tan_mag = (file_crv[mlType]).get_tan(0).mag();

  file_crv[mlType].load_tan( 0, top_tan*tan_mag );

  int last_id =  file_crv[mlType].get_num_pnts() - 1;
  vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
  tan_mag = file_crv[mlType].get_tan( last_id ).mag();
  file_crv[mlType].load_tan( last_id, bot_tan*tan_mag );

  fclose(file_id);

  return 1;


}

//==== Set/Copy File Curve From Pointer ====//
void fuse_xsec::set_file_crv( herm_curve* crv )
{
  file_crv[mlType] = *crv;
}

//==== Interpolate File Curve From Two Curves ====//
void fuse_xsec::interp_file_crv( double fract, herm_curve* crv0, herm_curve* crv1 )
{

  //==== Find Max Number of Curve Points ====//
  vec3d p0, p1, pnt;
  int num = MAX( crv0->get_num_pnts(), crv1->get_num_pnts() );

  file_crv[mlType].init( num );

  for ( int i = 0 ; i < num ; i++ )
  {
    float u = (float)i/(float)(num-1);

    p0 = crv0->comp_pnt_per_length( u );
    p1 = crv1->comp_pnt_per_length( u );

    pnt = p0 + (p1 - p0)*fract;
    file_crv[mlType].load_pnt( i, pnt );
  }

  file_crv[mlType].comp_tans();

  vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
  double tan_mag = file_crv[mlType].get_tan(0).mag();

  file_crv[mlType].load_tan( 0, top_tan*tan_mag );

  int last_id =  file_crv[mlType].get_num_pnts() - 1;
  vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
  tan_mag = file_crv[mlType].get_tan( last_id ).mag();
  file_crv[mlType].load_tan( last_id, bot_tan*tan_mag );


}

//===== Load Points from Curves ====//
void fuse_xsec::load_uniform_pnts( int mlt )
{
  int j;
  double y, z;
  int num_pnt_half = num_pnts/2 + 1;

  double l_top    = top_crv[mlt].get_length();
  double l_bot    = bot_crv[mlt].get_length();
  double l_circle = (double)(corner_rad[mlt]()*height[mlt]()*(PI - DEG_2_RAD*top_tan_angle[mlt]() - DEG_2_RAD*bot_tan_angle[mlt]()) );

  double total_length = l_top + l_bot + l_circle;

  double len = total_length/(double)(num_pnt_half-1);

  int cnt = 0;
//  double rem_len = 0.0;
  double rem_len = len;

  //==== Check For Point ====//
  if ( total_length < 0.000001 )
  {
    for ( j = 0 ; j <  num_pnt_half ; j++ )
    {
      pnts[mlt][cnt] = top_crv[mlt].comp_pnt_per_length(0.0);
      cnt++;
    }
  }
  //==== Top Crv Points ====//
  if ( l_top > 0.000001)
  {
    double u = 0.0;
    double del_u = len/l_top;

    while ( u <= 1.00000001 && cnt < num_pnt_half )
    {
      pnts[mlt][cnt] = top_crv[mlt].comp_pnt_per_length((float)u);

      cnt++;
      u += del_u;
    }
    rem_len = (1.0 - (u - del_u))*l_top;
  }
   //==== Circle Points ====//
   if ( l_circle > 0.000001 && rem_len < l_circle )
     {
       double top_ang =  PI/2.0 - DEG_2_RAD*top_tan_angle[mlt]();
       double bot_ang = -PI/2.0 + DEG_2_RAD*bot_tan_angle[mlt]();

       double ang_per_len = (top_ang - bot_ang)/l_circle;

       double ang = (len - rem_len)*ang_per_len;

       while ( (top_ang - ang) >= bot_ang  && cnt <  num_pnt_half )
       {
         double c_ang =  top_ang - ang;

         y = (double)( width[mlt]()/2.0 + corner_rad[mlt]()*height[mlt]()*(cos(c_ang) - 1.0) );
         z = (double)( max_width_loc[mlt]()*height[mlt]()/2.0 + corner_rad[mlt]()*height[mlt]()*sin(c_ang) );
         pnts[mlt][cnt] = vec3d( 0.0, y, z );
         cnt++;

         ang += len*ang_per_len;
       }
       ang -= len*ang_per_len;
	   if ( ang_per_len > 0.000001 )
			rem_len = ((top_ang - ang) - bot_ang)/ang_per_len;
	   else
		    rem_len = 0.0;
     }
  //==== Bot Crv Points ====//
  if ( l_bot > 0.000001)
  {
    double u = (len - rem_len)/l_bot;
    double del_u = len/l_bot;

    while ( u <= 1.000001 && cnt < num_pnt_half )
    {
      pnts[mlt][cnt] = bot_crv[mlt].comp_pnt_per_length((float)u);

      cnt++;
      u += del_u;
    }
  }

  //==== Fix Last Pnt ====//
  cnt = num_pnt_half-1;
  pnts[mlt][cnt] = bot_crv[mlt].comp_pnt_per_length(1.0);
  cnt++;

  //==== Reflection Points ====//
  int num_half_pnts = cnt;
  for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
    {
      pnts[mlt][cnt] = pnts[mlt][j].reflect_xz();
      cnt++;
    }

}


//===== Load Points from Curves ====//
void fuse_xsec::load_pnts( int mlt )
{
   int j;
   double u, y, z;
   int num_top, num_bot, num_circle;
   compute_pnt_distribution(mlt, num_top, num_circle, num_bot);

   int cnt = 0;

   //==== Top Curve Points ====//
   for ( j = 0 ; j < num_top ; j++ )
     {
       u = (double)j/(double)(num_top-1);
       pnts[mlt][cnt] = top_crv[mlt].comp_pnt_per_length((float)u);
       cnt++;
     }

   //==== Circle Points ====//
   if ( num_circle > 0 )
     {
       double delt_angle = (PI - DEG_2_RAD*top_tan_angle[mlt]() - DEG_2_RAD*bot_tan_angle[mlt]())/
                          (double)(num_circle + 1);

       for ( j = 0 ; j < num_circle ; j++)
         {
           double c_ang = PI/2.0 - DEG_2_RAD*top_tan_angle[mlt]() - delt_angle*(double)(j+1);
           y = (double)( width[mlt]()/2.0 + corner_rad[mlt]()*height[mlt]()*(cos(c_ang) - 1.0) );
           z = (double)( max_width_loc[mlt]()*height[mlt]()/2.0 + corner_rad[mlt]()*height[mlt]()*sin(c_ang) );
           pnts[mlt][cnt] = vec3d( 0.0, y, z );
           cnt++;
         }
     }

   //==== Bot Curve Points ====//
   for ( j = 0 ; j < num_bot ; j++ )
     {
       if ( (DEG_2_RAD*top_tan_angle[mlt]() + DEG_2_RAD*bot_tan_angle[mlt]()) > (PI/2.0 - 0.00001) )
         u = (double)(j+1)/(double)(num_bot);
       else
         u = (double)j/(double)(num_bot-1);

       pnts[mlt][cnt] = bot_crv[mlt].comp_pnt_per_length((float)u);
       cnt++;
     }

   //==== Reflection Points ====//
   int num_half_pnts = cnt;
   for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
     {
       pnts[mlt][cnt] = pnts[mlt][j].reflect_xz();
       cnt++;
     }

}

//===== Get Point  =====//
vec3d fuse_xsec::get_pnt(int index)
{
  return(pnts[mlType][index]);
}

//===== Get Point  =====//
vec3d fuse_xsec::get_pnt(int ml_type, int index)
{
  return(pnts[ml_type][index]);
}


//===== Compute Point Distribution =====//
void fuse_xsec::compute_pnt_distribution(int mlt, int& num_top, int& num_circle, int& num_bot)
{
  int num_pnt_half = num_pnts/2 + 1;

  double l_top    = top_crv[mlt].get_length();
  double l_bot    = bot_crv[mlt].get_length();

  double l_circle = (double)( corner_rad[mlt]()*height[mlt]()*(PI - DEG_2_RAD*top_tan_angle[mlt]() - DEG_2_RAD*bot_tan_angle[mlt]()) );

  double total_length = l_top + l_bot + l_circle;

  double circle_fract = 0;
  if ( total_length > DBL_EPSILON )
	  circle_fract = l_circle/total_length;

  if ( pntSpaceType == PNT_SPACE_FIXED )
  {
    //num_circle = MAX(2,num_pnt_half/8);
	  num_circle = (int)(circle_fract*(double)num_pnt_half);


    num_top    = (num_pnt_half - num_circle)/2 + 1;
    num_bot    = num_pnt_half - num_top - num_circle;

    if ( l_circle <= 0.000001 )
    {
      num_top += num_circle/2;
      num_circle = 0;
      num_bot = num_pnt_half - num_top - num_circle;
    }
  }
  else if ( pntSpaceType == PNT_SPACE_PER_XSEC )
  {
    if (total_length <= 0.000001 )
    {
      num_top    = num_pnt_half/3;
      num_circle = num_pnt_half/3;
      num_bot    = num_pnt_half - num_top - num_circle;
    }
    else
    {
	  num_circle = (int)(circle_fract*(double)num_pnt_half);
      num_top    = (int)((l_top/total_length)*(double)(num_pnt_half));
      //num_bot    = (int)((l_bot/total_length)*(double)num_pnt_half);
      //num_circle    = num_pnt_half - num_top - num_bot;

	  num_bot = num_pnt_half - num_circle - num_top;


      //if ( num_fract_circle < num_circle )
      //{
      //  num_top += num_circle/2;
      //  num_circle = 0;
      //  num_bot = num_pnt_half - num_top - num_circle;
      //}
    }
  }
}

double fuse_xsec::computeArea()
{
	vector< vec3d > pntVec;
	for ( int i = 0 ; i < num_pnts ; i++ )
	{
        vec3d pnt = pnts[0][i];
		pntVec.push_back( pnt );
	}

    vec3d zero;
	return poly_area( pntVec, zero );
}

void fuse_xsec::debug_print()
{
	int i, j;
	vec3d p;

	printf("\n\n");
	printf("fuse_xsec:: Geom Ptr %d\n", (long)geom_ptr );
	printf("fuse_xsec:: pnt_on_spine %f %f %f \n", pnt_on_spine.x(), pnt_on_spine.y(), pnt_on_spine.z() );
	printf("fuse_xsec:: iml_pnt_on_spine %f %f %f \n", iml_pnt_on_spine.x(), iml_pnt_on_spine.y(), iml_pnt_on_spine.z() );

	printf("fuse_xsec:: imlFlag %d\n", imlFlag );
	printf("fuse_xsec:: interpThickFlag %d\n", interpThickFlag );

	printf("fuse_xsec:: topThick %f\n",  topThick() );
	printf("fuse_xsec:: botThick %f\n",  botThick() );
	printf("fuse_xsec:: sideThick %f\n", sideThick() );
	printf("fuse_xsec:: actTopThick %f\n", actTopThick);
	printf("fuse_xsec:: actBotThick %f\n", actBotThick );
	printf("fuse_xsec:: actSideThick %f\n", actSideThick );
	printf("fuse_xsec:: imlXOff %f\n", imlXOff );
	printf("fuse_xsec:: imlZOff %f\n", imlZOff );

	printf("fuse_xsec:: mlType %d\n", mlType );
	printf("fuse_xsec:: z_offset %f\n", z_offset() );
	printf("fuse_xsec:: location %f\n", location() );

	for (  i = 0 ; i < 2 ; i++ )
	{
		printf("i = %d\n", i );
		printf("fuse_xsec:: height %f\n", height[i]() );
		printf("fuse_xsec:: width %f\n", width[i]() );
		printf("fuse_xsec:: max_width_loc %f\n", max_width_loc[i]() );
		printf("fuse_xsec:: corner_rad %f\n", corner_rad[i]() );
		printf("fuse_xsec:: top_tan_angle %f\n", top_tan_angle[i]() );
		printf("fuse_xsec:: bot_tan_angle %f\n", bot_tan_angle[i]() );
		printf("fuse_xsec:: top_str %f\n", top_str[i]() );
		printf("fuse_xsec:: upp_str %f\n", upp_str[i]() );
		printf("fuse_xsec:: low_str %f\n", low_str[i]() );
		printf("fuse_xsec:: bot_str %f\n", bot_str[i]() );
	}

	for (  i = 0 ; i < 2 ; i++ )
	{
		printf("i = %d\n", i );
		printf("  fuse_xsec:: allTanStrFlag %d\n", allTanStrFlag[i] );
		for (  j = 0 ; j < top_crv[i].get_num_pnts() ; j++ )
		{
			vec3d p = top_crv[i].get_pnt(j);
			printf("  fuse_xsec:: top_crv %f %f %f \n", p.x(), p.y(), p.z() );
		}
		for (  j = 0 ; j < bot_crv[i].get_num_pnts() ; j++ )
		{
			vec3d p = bot_crv[i].get_pnt(j);
			printf("  fuse_xsec:: bot_crv %f %f %f \n", p.x(), p.y(), p.z() );
		}
	}
	for (  i = 0 ; i < 2 ; i++ )
	{
		printf("i = %d\n", i );
		for (  j = 0 ; j < num_pnts ; j++ )
		{
			vec3d p = pnts[i][j];
			printf("  fuse_xsec:: pnts %f %f %f \n", p.x(), p.y(), p.z() );
		
		}
	}

}

