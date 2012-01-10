//******************************************************************************
//
//   Fusealge Cross-Section 2
//
//   J.R. Gloudemans - 8/18/2009
//      Extensively modified to create Fuse2
//
//   J.R. Gloudemans - 8/2/93
//   Sterling Software
//      Original fuselage fuse_xsec.cpp
//
//******************************************************************************
#include "FuselageXSec.h"
#include "FuselageGeom.h"

#include <math.h>
#include "util.h"
#include "defines.h"
#include "bezier_curve.h"

//===== Constructor  =====//
FuselageXSec::FuselageXSec(Geom* geom_ptr_in)
{
	geom_ptr = geom_ptr_in;

	edit_crv.set_geom( this );

	updateFuse.initialize(geom_ptr, UPD_FUSE_XSEC, "Update Fuse", 0.0 );

	pntSpaceType =PNT_SPACE_UNIFORM;

	drawScaleFactor   = 1.0;
	rollerScaleFactor = 2.0;
	setDrawScaleFactor( rollerScaleFactor );

	num_pnts = 33;

	y_offset.initialize(this, UPD_FUSE_XSEC, "Y_Offset", 0.0 );
	y_offset.set_lower_upper(-1000000.0, 1000000.0);
	y_offset.set_script("fuse_xsec yoffset", 0);

	z_offset.initialize(this, UPD_FUSE_XSEC, "Z_Offset", 0.0 );
	z_offset.set_lower_upper(-1000000.0, 1000000.0);
	z_offset.set_script("fuse_xsec zoffset", 0);

	location.initialize(this, UPD_FUSE_XSEC, "Location", 0.0 );
	location.set_lower_upper(0.0, 1.0);
	location.set_script("fuse_xsec location", 0);

	//==== Profile Stuff ====//
	topTanAng.initialize(this, UPD_FUSE_XSEC, "TopTanAng", 0.0 );
	topTanAng.set_lower_upper(-90.0, 90.0);
	topTanAng.set_script("fuse_xsec toptanang", 0); 
	topTanStr1.initialize(this, UPD_FUSE_XSEC, "TopTanStr1", 0.25 );
	topTanStr1.set_lower_upper(0.0, 1.0);
	topTanStr1.set_script("fuse_xsec toptanstr1", 0); 
	topTanStr2.initialize(this, UPD_FUSE_XSEC, "TopTanStr2", 0.25 );
	topTanStr2.set_lower_upper(0.0, 1.0);
	topTanStr2.set_script("fuse_xsec toptanstr2", 0); 

	botTanAng.initialize(this, UPD_FUSE_XSEC, "BotTanAng", 0.0 );
	botTanAng.set_lower_upper(-90.0, 90.0);
	botTanAng.set_script("fuse_xsec bottanang", 0); 
	botTanStr1.initialize(this, UPD_FUSE_XSEC, "BotTanStr1", 0.25 );
	botTanStr1.set_lower_upper(0.0, 1.0);
	botTanStr1.set_script("fuse_xsec bottanstr1", 0); 
	botTanStr2.initialize(this, UPD_FUSE_XSEC, "BotTanStr2", 0.25 );
	botTanStr2.set_lower_upper(0.0, 1.0);
	botTanStr2.set_script("fuse_xsec bottanstr2", 0); 

	leftTanAng.initialize(this, UPD_FUSE_XSEC, "LeftTanAng", 0.0 );
	leftTanAng.set_lower_upper(-90.0, 90.0);
	leftTanAng.set_script("fuse_xsec lefttanang", 0); 
	leftTanStr1.initialize(this, UPD_FUSE_XSEC, "LeftTanStr1", 0.25 );
	leftTanStr1.set_lower_upper(0.0, 1.0);
	leftTanStr1.set_script("fuse_xsec lefttanstr1", 0); 
	leftTanStr2.initialize(this, UPD_FUSE_XSEC, "LeftTanStr2", 0.25 );
	leftTanStr2.set_lower_upper(0.0, 1.0);
	leftTanStr2.set_script("fuse_xsec lefttanstr2", 0); 

	rightTanAng.initialize(this, UPD_FUSE_XSEC, "RightTanAng", 0.0 );
	rightTanAng.set_lower_upper(-90.0, 90.0);
	rightTanAng.set_script("fuse_xsec righttanang", 0); 
	rightTanStr1.initialize(this, UPD_FUSE_XSEC, "RightTanStr1", 0.25 );
	rightTanStr1.set_lower_upper(0.0, 1.0);
	rightTanStr1.set_script("fuse_xsec righttanstr1", 0); 
	rightTanStr2.initialize(this, UPD_FUSE_XSEC, "RightTanStr2", 0.25 );
	rightTanStr2.set_lower_upper(0.0, 1.0);
	rightTanStr2.set_script("fuse_xsec righttanstr2", 0); 

	topSymFlag = 0;
	sideSymFlag = 1;

	top_crv.init(2);
	bot_crv.init(2);

	pnts.init(num_pnts);

	height.initialize(this, UPD_FUSE_XSEC, "Height", 3.0 );
	height.set_lower_upper(0.0, 10000000.0);
	height.set_script("fuse_xsec height", 0);

	width.initialize(this, UPD_FUSE_XSEC, "Width", 2.5 );
	width.set_lower_upper(0.0, 10000000.0);
	width.set_script("fuse_xsec width", 0);

	max_width_loc.initialize(this, UPD_FUSE_XSEC, "Max_Width_Location", 0.0 );
	max_width_loc.set_lower_upper(-1000000.0, 1000000.0);
	max_width_loc.set_script("fuse_xsec widthoffset", 0);

	corner_rad.initialize(this, UPD_FUSE_XSEC, "Corner_Radius", 0.1 );
	corner_rad.set_lower_upper(0.0001, 1000000.0);
	corner_rad.set_script("fuse_xsec cornerrad", 0);

	top_tan_angle.initialize(this, UPD_FUSE_XSEC, "Top_Tan_Angle", 90.0 );
	top_tan_angle.set_lower_upper(0.0, 90.0);
	top_tan_angle.set_script("fuse_xsec toptanang", 0);

	bot_tan_angle.initialize(this, UPD_FUSE_XSEC, "Bot_Tan_Angle", 90.0 );
	bot_tan_angle.set_lower_upper(0.0, 90.0);
	bot_tan_angle.set_script("fuse_xsec bottanang", 0);

	top_str.initialize(this, UPD_FUSE_XSEC, "TopStr", 0.50 );
	top_str.set_lower_upper(0.001, 2.0);
	top_str.set_script("fuse_xsec alltanstr", 0); // not a bug

	upp_str.initialize(this, UPD_FUSE_XSEC, "UppStr", 0.50 );
	upp_str.set_lower_upper(0.001, 2.0);
	upp_str.set_script("fuse_xsec upcornertanstr", 0);

	low_str.initialize(this, UPD_FUSE_XSEC, "LowStr", 0.50 );
	low_str.set_lower_upper(0.001, 2.0);
	low_str.set_script("fuse_xsec lowcornertanstr", 0);

	bot_str.initialize(this, UPD_FUSE_XSEC, "BotStr", 0.50 );
	bot_str.set_lower_upper(0.001, 2.0);
	bot_str.set_script("fuse_xsec bottanstr", 0);

	allTanStrFlag = 0;

	set_type( FXS_ELLIPSE );

	numSectInterp1 = 5;
	numSectInterp2 = 5;

	this->gen_parms();
	this->generate();


}

void FuselageXSec::set_fuse_ptr( Geom* geom_ptr_in )
{
	geom_ptr = geom_ptr_in;

	updateFuse.set_geom(geom_ptr);

	y_offset.set_geom( this );
	z_offset.set_geom( this );
	location.set_geom( this );

	topTanStr1.set_geom( this );
	topTanStr2.set_geom( this );
	topTanAng.set_geom( this );
	botTanStr1.set_geom( this );
	botTanStr2.set_geom( this );
	botTanAng.set_geom( this );
	leftTanStr1.set_geom( this );
	leftTanStr2.set_geom( this );
	leftTanAng.set_geom( this );
	rightTanStr1.set_geom( this );
	rightTanStr2.set_geom( this );
	rightTanAng.set_geom( this );

	height.set_geom( this );
	width.set_geom( this );
	max_width_loc.set_geom( this );
	corner_rad.set_geom( this );
	top_tan_angle.set_geom( this );
	bot_tan_angle.set_geom( this );
	top_str.set_geom( this );
	upp_str.set_geom( this );
	low_str.set_geom( this );
	bot_str.set_geom( this );

	edit_crv.set_geom( this );
}

//===== Destructor  =====//
FuselageXSec::~FuselageXSec()
{
}

////===== Copy - Override Equals  =====//
//FuselageXSec& FuselageXSec::operator=(const FuselageXSec& ifx)
//{
//
////	copy( ifx );
//	return *this;  
//}

void FuselageXSec::copy( FuselageXSec & ifx )
{
	num_pnts = ifx.num_pnts;
	y_offset = ifx.y_offset.get();
	z_offset = ifx.z_offset.get();
	pntSpaceType = ifx.pntSpaceType;

	topTanAng  = ifx.topTanAng.get();
	topTanStr1 = ifx.topTanStr1.get();
	topTanStr2 = ifx.topTanStr2.get();

	botTanAng  = ifx.botTanAng.get();
	botTanStr1 = ifx.botTanStr1.get();
	botTanStr2 = ifx.botTanStr2.get();

	leftTanAng  = ifx.leftTanAng.get();
	leftTanStr1 = ifx.leftTanStr1.get();
	leftTanStr2 = ifx.leftTanStr2.get();

	rightTanAng  = ifx.rightTanAng.get();
	rightTanStr1 = ifx.rightTanStr1.get();
	rightTanStr2 = ifx.rightTanStr2.get();

	topSymFlag  = ifx.topSymFlag;
	sideSymFlag = ifx.sideSymFlag;
	allTanStrFlag  = 0;

	numSectInterp1 = ifx.numSectInterp1;
	numSectInterp2 = ifx.numSectInterp2;

	xstype = ifx.xstype;
	height = ifx.height.get();
	width  = ifx.width.get();
	max_width_loc = ifx.max_width_loc.get();
	corner_rad    = ifx.corner_rad.get();
	top_tan_angle = ifx.top_tan_angle.get();
	bot_tan_angle = ifx.bot_tan_angle.get();
	top_str = ifx.top_str.get();
	upp_str = ifx.upp_str.get();
	low_str = ifx.low_str.get();
	bot_str = ifx.bot_str.get();
	file_crv = ifx.file_crv;
	edit_crv = ifx.edit_crv;

	edit_crv .set_geom( this );
	edit_crv .generate();

	this->gen_parms();
	this->generate();
}


void FuselageXSec::setAllTanStrFlag( int f )		
{ 
	allTanStrFlag = f; 

	if (f)
		top_str.set_script("fuse_xsec alltanstr", 0);
	else
		top_str.set_script("fuse_xsec toptanstr", 0);

}


//==== Draw Cross Section In 2D Window ====//
void FuselageXSec::draw()
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


	glColor3f(1.0, 0.0, 0.0);
	glBegin( GL_LINE_STRIP );
    for (  i = 0 ; i < num_pnts ; i++)
    {
		vec3d p = pnts[i];
		glVertex2d( p.y(), p.z() );
	}
	glEnd();


	glColor3f(0.0, 0.0, 1.0);
	glBegin( GL_LINES );
		
	vec3d p = top_crv.get_pnt(0);
	glVertex2d( p.y(), p.z() );
	p = p + top_crv.get_tan(0);
	glVertex2d( p.y(), p.z() );

	p = top_crv.get_pnt(1);
	glVertex2d( p.y(), p.z() );
	p = p - top_crv.get_tan(1);
	glVertex2d( p.y(), p.z() );
	
	p = bot_crv.get_pnt(0);
	glVertex2d( p.y(), p.z() );
	p = p + bot_crv.get_tan(0);
	glVertex2d( p.y(), p.z() );

	p = bot_crv.get_pnt(1);
	glVertex2d( p.y(), p.z() );
	p = p - bot_crv.get_tan(1);
	glVertex2d( p.y(), p.z() );

	glEnd();
 

	glPopMatrix();

}

//==== Parm Has Changed ReGenerate Component ====//
void FuselageXSec::parm_changed(Parm* p)
{
	if ( allTanStrFlag == 1 && xstype !=  FXS_GENERAL )
	{
		upp_str = low_str = bot_str =  top_str();
	}

	regenerate();

	//==== Trigger Fuse Geom Update ====//
	if ( geom_ptr )
		geom_ptr->parm_changed( &updateFuse );
}


//==== Set Type Of Cross Section  ====//
void FuselageXSec::set_type(int type_in)           
{ 
 	xstype = type_in; 

	//==== Deactivate Everything ====//
	height.deactivate();
	width.deactivate();
	max_width_loc.deactivate();
	corner_rad.deactivate();
	top_tan_angle.deactivate();
	bot_tan_angle.deactivate();
	top_str.deactivate();
	upp_str.deactivate();
	low_str.deactivate();
	bot_str.deactivate();

	if ( xstype == FXS_POINT )
	{
	}
	else if ( xstype == FXS_CIRCLE )
	{
		height.activate();
	}
	else if ( xstype == FXS_ELLIPSE )
	{
		height.activate();
		width.activate();
	}
	else if ( xstype == FXS_BOX )
	{
		height.activate();
		width.activate();
	}
	else if ( xstype == FXS_RND_BOX )
	{
		height.activate();
		width.activate();
		top_str.activate();
	}
	else if ( xstype == FXS_GENERAL )
	{
		height.activate();
		width.activate();
		max_width_loc.activate();
		corner_rad.activate();
		top_tan_angle.activate();
		bot_tan_angle.activate();
		top_str.activate();
		upp_str.activate();
		low_str.activate();
		bot_str.activate();
	}
	else if ( xstype == FXS_FROM_FILE )
	{
		height.activate();
		width.activate();
	}
	else if ( xstype == FXS_EDIT_CRV )
	{
	}
}

void FuselageXSec::setDrawScaleFactor( double sf )
{
	rollerScaleFactor = sf;
	drawScaleFactor = (double)pow(10.0, rollerScaleFactor)/1000.0;
}

//==== Set Point On Span Offset by ZOffset  ====//
void FuselageXSec::set_pnt_on_spine(const vec3d& offset)
{
  pnt_on_spine = offset;
  pnt_on_spine.offset_y(y_offset() );
  pnt_on_spine.offset_z(z_offset() );
}

void FuselageXSec::setEditCrv( EditCurve* crv )
{ 
	edit_crv = *crv; 
	edit_crv.set_geom( geom_ptr );
}

//==== Set NUmber of Points ====//
void FuselageXSec::set_num_pnts(int num_pnts_in)
{
	if ( num_pnts_in > 8 )
	{
		int corr_num_pnts = 8*(num_pnts_in/8)+1;
		num_pnts = corr_num_pnts;

		pnts.init(num_pnts);
		this->gen_parms();
		this->generate();
	}
}


//==== Write Fuse Cross Section File ====//
void FuselageXSec::write(xmlNodePtr root)
{
  xmlAddIntNode( root, "Num_Pnts", num_pnts);
  xmlAddDoubleNode( root, "Spine_Location", location() );
  xmlAddDoubleNode( root, "Y_Offset", y_offset());
  xmlAddDoubleNode( root, "Z_Offset", z_offset());

  //==== Profile Stuff ====//
  xmlAddDoubleNode( root, "Top_Tan_Ang",   topTanAng());
  xmlAddDoubleNode( root, "Top_Tan_Str_1", topTanStr1());
  xmlAddDoubleNode( root, "Top_Tan_Str_2", topTanStr2());

  xmlAddDoubleNode( root, "Bot_Tan_Ang",   botTanAng());
  xmlAddDoubleNode( root, "Bot_Tan_Str_1", botTanStr1());
  xmlAddDoubleNode( root, "Bot_Tan_Str_2", botTanStr2());

  xmlAddDoubleNode( root, "Left_Tan_Ang",   leftTanAng());
  xmlAddDoubleNode( root, "Left_Tan_Str_1", leftTanStr1());
  xmlAddDoubleNode( root, "Left_Tan_Str_2", leftTanStr2());

  xmlAddDoubleNode( root, "Right_Tan_Ang",   rightTanAng());
  xmlAddDoubleNode( root, "Right_Tan_Str_1", rightTanStr1());
  xmlAddDoubleNode( root, "Right_Tan_Str_2", rightTanStr2());

  xmlAddIntNode( root, "Num_Sect_Interp_1", numSectInterp1);
  xmlAddIntNode( root, "Num_Sect_Interp_2", numSectInterp2);

  xmlAddIntNode( root, "Top_Sym_Flag",  topSymFlag);
  xmlAddIntNode( root, "Side_Sym_Flag", sideSymFlag);

  write_parms( root );
}

//==== Write Fuse Cross Section File ====//
void FuselageXSec::write_parms(xmlNodePtr root )
{
	xmlAddIntNode( root, "Type", xstype);

	xmlAddDoubleNode( root, "Height", height() );
	xmlAddDoubleNode( root, "Width",  width() );

	xmlAddDoubleNode( root, "Max_Width_Location", max_width_loc() );
	xmlAddDoubleNode( root, "Corner_Radius", corner_rad() );
	xmlAddDoubleNode( root, "Top_Tan_Angle", top_tan_angle() );
	xmlAddDoubleNode( root, "Bot_Tan_Angle", bot_tan_angle() );

	xmlAddDoubleNode( root, "Top_Tan_Strength",    top_str() );
	xmlAddDoubleNode( root, "Upper_Tan_Strength",  upp_str() );
	xmlAddDoubleNode( root, "Lower_Tan_Strength",  low_str() );
	xmlAddDoubleNode( root, "Bottom_Tan_Strength", bot_str() );

	if ( xstype == FXS_FROM_FILE )
	{
		vec3d pnt;
		Stringc ystr, zstr;
		char numstr[255];
		int num_pnts = file_crv.get_num_pnts();

		xmlAddStringNode( root, "File_Name", fileName );
		for ( int i = 0 ; i < file_crv.get_num_pnts() ; i++ )
		{
			pnt = file_crv.get_pnt( i );
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
	else if ( xstype == FXS_EDIT_CRV )
	{
		edit_crv.write( root );
	}
}

//==== Read Fuse Cross Section File ====//
void FuselageXSec::read(xmlNodePtr node)
{
	num_pnts = xmlFindInt( node, "Num_Pnts", num_pnts );
	location = xmlFindDouble( node, "Spine_Location", location() );
	y_offset =  xmlFindDouble( node, "Y_Offset", y_offset() );
	z_offset =  xmlFindDouble( node, "Z_Offset", z_offset() );

	//==== Profile Stuff ====//
	topTanAng    = xmlFindDouble( node, "Top_Tan_Ang",   topTanAng() );
	topTanStr1   = xmlFindDouble( node, "Top_Tan_Str_1", topTanStr1() );
	topTanStr2   = xmlFindDouble( node, "Top_Tan_Str_2", topTanStr2() );

	botTanAng    = xmlFindDouble( node, "Bot_Tan_Ang",   botTanAng() );
	botTanStr1   = xmlFindDouble( node, "Bot_Tan_Str_1", botTanStr1() );
	botTanStr2   = xmlFindDouble( node, "Bot_Tan_Str_2", botTanStr2() );

	leftTanAng    = xmlFindDouble( node, "Left_Tan_Ang",   leftTanAng() );
	leftTanStr1   = xmlFindDouble( node, "Left_Tan_Str_1", leftTanStr1() );
	leftTanStr2   = xmlFindDouble( node, "Left_Tan_Str_2", leftTanStr2() );

	rightTanAng    = xmlFindDouble( node, "Right_Tan_Ang",   rightTanAng() );
	rightTanStr1   = xmlFindDouble( node, "Right_Tan_Str_1", rightTanStr1() );
	rightTanStr2   = xmlFindDouble( node, "Right_Tan_Str_2", rightTanStr2() );

	numSectInterp1  = xmlFindInt( node, "Num_Sect_Interp_1", numSectInterp1 );
	numSectInterp2  = xmlFindInt( node, "Num_Sect_Interp_2", numSectInterp2 );

	topSymFlag  = xmlFindInt( node, "Top_Sym_Flag", topSymFlag );
	sideSymFlag  = xmlFindInt( node, "Side_Sym_Flag", sideSymFlag );

	read_parms( node );
}

//==== Read Fuse Cross Section File ====//
void FuselageXSec::read_parms(xmlNodePtr node)
{
	int i;
	xstype = xmlFindInt( node, "Type", xstype );

	height =  xmlFindDouble( node, "Height", height() );
	width  =  xmlFindDouble( node, "Width",  width() );

	max_width_loc  = xmlFindDouble( node, "Max_Width_Location",  max_width_loc() );
	corner_rad     = xmlFindDouble( node, "Corner_Radius",  corner_rad() );
	top_tan_angle = xmlFindDouble( node, "Top_Tan_Angle", top_tan_angle());
	bot_tan_angle = xmlFindDouble( node, "Bot_Tan_Angle", bot_tan_angle());

	top_str  = xmlFindDouble( node, "Top_Tan_Strength",    top_str() );
	upp_str  = xmlFindDouble( node, "Upper_Tan_Strength",  upp_str() );
	low_str  = xmlFindDouble( node, "Lower_Tan_Strength",  low_str() );
	bot_str  = xmlFindDouble( node, "Bottom_Tan_Strength", bot_str() );

	if ( xstype == FXS_FROM_FILE )
	{
		xstype = FXS_GENERAL;				// If this fails fall back on something...

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

				xstype = FXS_FROM_FILE;	

				file_crv.init( num_pnts );
				for ( i = 0 ; i < num_pnts ; i++ )
				{
					file_crv.load_pnt( i, vec3d( 0.0, arry[i],  arrz[i] ));
				}
				free( arry );
				free( arrz );

				file_crv.comp_tans();

				vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
				double tan_mag = file_crv.get_tan(0).mag();

				file_crv.load_tan( 0, top_tan*tan_mag );

				int last_id =  file_crv.get_num_pnts() - 1;
				vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
				tan_mag = file_crv.get_tan( last_id ).mag();
				file_crv.load_tan( last_id, bot_tan*tan_mag );
			}
		}
	}
	else if ( xstype == FXS_EDIT_CRV )
	{
		edit_crv.read( node );
	}
	set_type(xstype);  

}

//===== Regenerate Current Fuse Cross Section And Fuse  =====//
void FuselageXSec::regenerate()
{
  gen_parms();
  generate();
}

//===== Generate All Parmeters For X-Sec  =====//
void FuselageXSec::gen_parms()
{
   switch ( xstype )
    {
      case FXS_POINT:
        height = 0.0;
        width  = 0.0;
        max_width_loc = 0.0;
        corner_rad = 0.0;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        top_str = upp_str = low_str = bot_str = 0.83;
      break;

      case FXS_CIRCLE:
        width  = height();
        max_width_loc = 0.0;
        corner_rad = 0.5;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        top_str = upp_str = low_str = bot_str = .83;
      break;

      case FXS_ELLIPSE:
        max_width_loc = 0.0;
        corner_rad = 0.5;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        top_str = upp_str = low_str = bot_str = .83;
      break;

      case FXS_FROM_FILE:
        max_width_loc = 0.0;
        corner_rad = 0.5;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        top_str = upp_str = low_str = bot_str = .83;
      break;

      case FXS_BOX:
        max_width_loc = 0.0;
        corner_rad = 0.5;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        top_str = upp_str = low_str = bot_str = .83;
      break;

	  case FXS_RND_BOX:
        max_width_loc = 0.0;
        top_tan_angle = 90.0;
        bot_tan_angle = 90.0;
        corner_rad = 0.5;
      break;

	  case FXS_EDIT_CRV:
	  break;
   }
}


//===== Generate  Xsec  =====//
void FuselageXSec::generate()
{
    double y, z;
	if ( xstype == FXS_FROM_FILE )
	{
	 pnts.init(num_pnts);
	 load_file_pnts();
	}
	else if ( xstype == FXS_EDIT_CRV )
	{
	 pnts.init(num_pnts);
	 load_edit_crv_pnts();
	}
	else if ( xstype == FXS_BOX )
	{
	  pnts.init(num_pnts);
	  load_box_pnts();
	}
	else
	{
		if ( xstype != FXS_GENERAL )
		{
			upp_str = low_str = bot_str =  top_str();
		}

		y = 0.0;
		z = height()/2.0;
		top_crv.load_pnt( 0, vec3d( 0.0, y, z ));

		y = (double)(top_str()*width());
		z = 0.0;
		top_crv.load_tan( 0, vec3d( 0.0, y, z ));

		y = (double)( width()/2.0 + corner_rad()*height()*(sin(DEG_2_RAD*top_tan_angle())-1.0) );
		z = (double)(max_width_loc()*height()/2.0 + corner_rad()*height()*cos(DEG_2_RAD*top_tan_angle()) ) ;
		top_crv.load_pnt( 1, vec3d( 0.0, y, z ));

		y = (double)( upp_str()*height()*cos(DEG_2_RAD*top_tan_angle()) );
		z = (double)( -upp_str()*height()*sin(DEG_2_RAD*top_tan_angle()) );
		top_crv.load_tan( 1, vec3d( 0.0, y, z ));

		y = (double)(width()/2.0 + corner_rad()*height()*(sin(DEG_2_RAD*bot_tan_angle())-1.0) );
		z = (double)(max_width_loc()*height()/2.0 - corner_rad()*height()*cos(DEG_2_RAD*bot_tan_angle()) );
		bot_crv.load_pnt( 0, vec3d( 0.0, y, z ));

		y = (double)( -low_str()*height()*cos(DEG_2_RAD*bot_tan_angle()) );
		z = (double)( -low_str()*height()*sin(DEG_2_RAD*bot_tan_angle()) );
		bot_crv.load_tan( 0, vec3d( 0.0, y, z ));

		y = 0.0;
		z = (double)( -height()/2.0 );
		bot_crv.load_pnt( 1, vec3d( 0.0, y, z ));

		y = (double)( -bot_str()*width() );
		z = 0.0;
		bot_crv.load_tan( 1, vec3d( 0.0, y, z ));

		pnts.init(num_pnts);
		if ( pntSpaceType == PNT_SPACE_FIXED || pntSpaceType == PNT_SPACE_PER_XSEC )
		{
			load_pnts();
		}
		else if ( pntSpaceType == PNT_SPACE_UNIFORM )
		{
			load_uniform_pnts();
		}
	}
}

//===== Load Points from File Curve ====//
void FuselageXSec::load_file_pnts()
{
	int j;
	double u;
	double zfact = height()/2.0;
	double yfact = width()/2.0;
	int num_pnt_half = num_pnts/2 + 1;
	int cnt = 0;

	//==== Top Curve Points ====//
	for ( j = 0 ; j < num_pnt_half ; j++ )
	{
		u = (double)j/(double)(num_pnt_half-1);
		pnts[cnt] = file_crv.comp_pnt_per_length((float)u);
		pnts[cnt].scale_y( yfact );
		pnts[cnt].scale_z( zfact );
		cnt++;
	}

	//==== Reflection Points ====//
	int num_half_pnts = cnt;
	for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
	{
		pnts[cnt] = pnts[j].reflect_xz();
		cnt++;
	}
}

//===== Load Points for Box ====//
void FuselageXSec::load_box_pnts( )
{
	vector< vec3d > pnt_vec;

	double y = height()/2.0;
	double x = width()/2.0;
	double f = 0.25;
	double g = 0.75;

	pnt_vec.push_back( vec3d( 0, y, 0 ) );
	pnt_vec.push_back( vec3d( f*x, y, 0 ) );
	pnt_vec.push_back( vec3d( g*x, y, 0 ) );

	pnt_vec.push_back( vec3d( x, y, 0 ) );
	pnt_vec.push_back( vec3d( x, g*y, 0 ) );
	pnt_vec.push_back( vec3d( x, f*y, 0 ) );

	pnt_vec.push_back( vec3d( x, 0, 0 ) );
	pnt_vec.push_back( vec3d( x, -f*y, 0 ) );
	pnt_vec.push_back( vec3d( x, -g*y, 0 ) );

	pnt_vec.push_back( vec3d( x, -y, 0 ) );
	pnt_vec.push_back( vec3d( g*x, -y, 0 ) );
	pnt_vec.push_back( vec3d( f*x, -y, 0 ) );

	pnt_vec.push_back( vec3d( 0, -y, 0 ) );
	pnt_vec.push_back( vec3d( -f*x, -y, 0 ) );
	pnt_vec.push_back( vec3d( -g*x, -y, 0 ) );

	pnt_vec.push_back( vec3d( -x, -y, 0 ) );
	pnt_vec.push_back( vec3d( -x, -g*y, 0 ) );
	pnt_vec.push_back( vec3d( -x, -f*y, 0 ) );

	pnt_vec.push_back( vec3d( -x,  0, 0 ) );
	pnt_vec.push_back( vec3d( -x, f*y, 0 ) );
	pnt_vec.push_back( vec3d( -x, g*y, 0 ) );

	pnt_vec.push_back( vec3d( -x, y, 0 ) );
	pnt_vec.push_back( vec3d( -g*x, y, 0 ) );
	pnt_vec.push_back( vec3d( -f*x, y, 0 ) );

	pnt_vec.push_back( vec3d( 0, y, 0 ) );

	Bezier_curve crv;
	
	crv.init_num_sections( 8 );
	for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
		crv.put_pnt( i, pnt_vec[i] );

	vec3d p;
	for ( int i = 0 ; i < num_pnts ; i++ )
	{
		float u = (float)i/(float)(num_pnts-1);
		p = crv.comp_pnt(u);
        pnts[i] = vec3d( 0.0, p.x(), p.y() );
	}

}

void FuselageXSec::load_edit_crv_pnts( )
{
	vec3d p;
	Bezier_curve crv = edit_crv.getBezierCurve();

	for ( int i = 0 ; i < num_pnts ; i++ )
	{
		float u = (float)i/(float)(num_pnts-1);
		p = crv.comp_pnt(u);
        pnts[i] = vec3d( 0.0, p.x(), p.y() );
	}
}

//===== Read AF File =====//
int FuselageXSec::read_xsec_file( Stringc file_name )
{
  int i;
  Stringc line;
  char buff[255];

  //==== Open file ====//
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

  file_crv.init( num_pnts );
  for ( i = 0 ; i < num_pnts ; i++ )
  {
    fscanf(file_id, "%f  %f", &y, &z);  fgets(buff, 80, file_id);
    file_crv.load_pnt( i, vec3d( 0.0, y,  z ));
  }

  //==== Find Height & Width ====//
  vec3d p0 = file_crv.get_pnt( 0 );
  vec3d pn = file_crv.get_pnt( num_pnts-1 );
  height = fabs(p0.z() - pn.z());

  double max_y = 0;
  for ( i = 0 ; i < num_pnts ; i++ )
  {
    double y = file_crv.get_pnt( i ).y();
    if  ( fabs(y) > max_y )
      max_y = fabs(y);
  }
  width = (2.0*max_y);

  //==== Scale Point By Height & Width ====//
  for ( i = 0 ; i < num_pnts ; i++ )
  {
    double y = 2.0*file_crv.get_pnt( i ).y()/width();
    double z = 2.0*file_crv.get_pnt( i ).z()/height();
    file_crv.load_pnt( i, vec3d( 0.0, y,  z ));
  }

  file_crv.comp_tans();
  vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
  double tan_mag = (file_crv).get_tan(0).mag();

  file_crv.load_tan( 0, top_tan*tan_mag );
  int last_id =  file_crv.get_num_pnts() - 1;
  vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
  tan_mag = file_crv.get_tan( last_id ).mag();
  file_crv.load_tan( last_id, bot_tan*tan_mag );

  fclose(file_id);

  return 1;
}

//==== Set/Copy File Curve From Pointer ====//
void FuselageXSec::set_file_crv( herm_curve* crv )
{
  file_crv = *crv;
}

//==== Interpolate File Curve From Two Curves ====//
void FuselageXSec::interp_file_crv( double fract, herm_curve* crv0, herm_curve* crv1 )
{
  //==== Find Max Number of Curve Points ====//
  vec3d p0, p1, pnt;
  int num = MAX( crv0->get_num_pnts(), crv1->get_num_pnts() );

  file_crv.init( num );

  for ( int i = 0 ; i < num ; i++ )
  {
    float u = (float)i/(float)(num-1);
    p0 = crv0->comp_pnt_per_length( u );
    p1 = crv1->comp_pnt_per_length( u );
    pnt = p0 + (p1 - p0)*fract;
    file_crv.load_pnt( i, pnt );
  }

  file_crv.comp_tans();

  vec3d top_tan = vec3d( 0.0, 1.0, 0.0 );
  double tan_mag = file_crv.get_tan(0).mag();

  file_crv.load_tan( 0, top_tan*tan_mag );

  int last_id =  file_crv.get_num_pnts() - 1;
  vec3d bot_tan = vec3d( 0.0, -1.0, 0.0 );
  tan_mag = file_crv.get_tan( last_id ).mag();
  file_crv.load_tan( last_id, bot_tan*tan_mag );
}

//===== Load Points from Curves ====//
void FuselageXSec::load_uniform_pnts()
{
  int j;
  double y, z;
  int num_pnt_half = num_pnts/2 + 1;

  double l_top    = top_crv.get_length();
  double l_bot    = bot_crv.get_length();
  double l_circle = (double)(corner_rad()*height()*(PI - DEG_2_RAD*top_tan_angle() - DEG_2_RAD*bot_tan_angle()) );

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
      pnts[cnt] = top_crv.comp_pnt_per_length(0.0);
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
      pnts[cnt] = top_crv.comp_pnt_per_length((float)u);

      cnt++;
      u += del_u;
    }
    rem_len = (1.0 - (u - del_u))*l_top;
  }
   //==== Circle Points ====//
   if ( l_circle > 0.000001 && rem_len < l_circle )
     {
       double top_ang =  PI/2.0 - DEG_2_RAD*top_tan_angle();
       double bot_ang = -PI/2.0 + DEG_2_RAD*bot_tan_angle();

       double ang_per_len = (top_ang - bot_ang)/l_circle;

       double ang = (len - rem_len)*ang_per_len;

       while ( (top_ang - ang) >= bot_ang  && cnt <  num_pnt_half )
       {
         double c_ang =  top_ang - ang;

         y = (double)( width()/2.0 + corner_rad()*height()*(cos(c_ang) - 1.0) );
         z = (double)( max_width_loc()*height()/2.0 + corner_rad()*height()*sin(c_ang) );
         pnts[cnt] = vec3d( 0.0, y, z );
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
      pnts[cnt] = bot_crv.comp_pnt_per_length((float)u);

      cnt++;
      u += del_u;
    }
  }

  //==== Fix Last Pnt ====//
  cnt = num_pnt_half-1;
  pnts[cnt] = bot_crv.comp_pnt_per_length(1.0);
  cnt++;

  //==== Reflection Points ====//
  int num_half_pnts = cnt;
  for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
    {
      pnts[cnt] = pnts[j].reflect_xz();
      cnt++;
    }
}

//===== Load Points from Curves ====//
void FuselageXSec::load_pnts()
{
   int j;
   double u, y, z;
   int num_top, num_bot, num_circle;
   compute_pnt_distribution( num_top, num_circle, num_bot );

   int cnt = 0;

   //==== Top Curve Points ====//
   for ( j = 0 ; j < num_top ; j++ )
     {
       u = (double)j/(double)(num_top-1);
       pnts[cnt] = top_crv.comp_pnt_per_length((float)u);
       cnt++;
     }

   //==== Circle Points ====//
   if ( num_circle > 0 )
     {
       double delt_angle = (PI - DEG_2_RAD*top_tan_angle() - DEG_2_RAD*bot_tan_angle())/
                          (double)(num_circle + 1);

       for ( j = 0 ; j < num_circle ; j++)
         {
           double c_ang = PI/2.0 - DEG_2_RAD*top_tan_angle() - delt_angle*(double)(j+1);
           y = (double)( width()/2.0 + corner_rad()*height()*(cos(c_ang) - 1.0) );
           z = (double)( max_width_loc()*height()/2.0 + corner_rad()*height()*sin(c_ang) );
           pnts[cnt] = vec3d( 0.0, y, z );
           cnt++;
         }
     }

   //==== Bot Curve Points ====//
   for ( j = 0 ; j < num_bot ; j++ )
     {
       if ( (DEG_2_RAD*top_tan_angle() + DEG_2_RAD*bot_tan_angle()) > (PI/2.0 - 0.00001) )
         u = (double)(j+1)/(double)(num_bot);
       else
         u = (double)j/(double)(num_bot-1);

       pnts[cnt] = bot_crv.comp_pnt_per_length((float)u);
       cnt++;
     }

   //==== Reflection Points ====//
   int num_half_pnts = cnt;
   for ( j = num_half_pnts-2 ; j >= 0 ; j-- )
     {
       pnts[cnt] = pnts[j].reflect_xz();
       cnt++;
     }
}

//===== Get Point  =====//
vec3d FuselageXSec::get_pnt(int index)
{
  return(pnts[index]);
}

double FuselageXSec::linear_interp( int imin, int ival, int imax, double vmin, double vmax )
{
	double val = vmin;
	double denom = (double)(imax - imin);
	if ( denom > DBL_EPSILON )
	{
		double fract = (double)(ival - imin)/denom;
		val = vmin + fract*(vmax - vmin);
	}
	return val;
}

double FuselageXSec::cosine_interp( int imin, int ival, int imax, double vmin, double vmax )
{
	double val = vmin;
	double denom = (double)(imax - imin);
	if ( denom > DBL_EPSILON )
	{
		double fract = (double)(ival - imin)/denom;
		double mu2 = (1.0-cos(fract*PI))/2.0;
		val = vmin + mu2*(vmax - vmin);
	}
	return val;
}

double FuselageXSec::cubic_interp( int imin, int ival, int imax, double vmin, double vmax )
{
	double val = vmin;
	double denom = (double)(imax - imin);
	if ( denom > DBL_EPSILON )
	{
		double u = (double)(ival - imin)/denom;
		double u2 = u*u;
		double u3 = u2*u;
		val = vmin*(2.0*u3 - 3.0*u2 + 1.0) + vmax*(3.0*u2 - 2.0*u3);
	}
	return val;
}

//===== Get Tan Ang (Interpolate) =====//
double FuselageXSec::get_tan_ang( int index )
{
	int q2 = num_pnts/2;
	int q1 = q2/2;
	int q3 = q2 + q1;
	int q4 = num_pnts-1;

	double val = 0.0;
	if ( index <= q1 )
		val = cubic_interp( 0,  index, q1, topTanAng(), rightTanAng() ); 
	else if ( index <= q2 )
		val = cubic_interp( q1, index, q2, rightTanAng(), botTanAng() ); 
	else if ( index <= q3 )
		val = cubic_interp( q2, index, q3, botTanAng(), leftTanAng() ); 
	else
		val = cubic_interp( q3, index, q4, leftTanAng(), topTanAng() ); 

	return val;
}
  
double FuselageXSec::get_tan_str1( int index )
{
	int q2 = num_pnts/2;
	int q1 = q2/2;
	int q3 = q2 + q1;
	int q4 = num_pnts-1;

	double val = 0.0;
	if ( index <= q1 )
		val = cubic_interp( 0,  index, q1, topTanStr1(), rightTanStr1() ); 
	else if ( index <= q2 )
		val = cubic_interp( q1, index, q2, rightTanStr1(), botTanStr1() ); 
	else if ( index <= q3 )
		val = cubic_interp( q2, index, q3, botTanStr1(), leftTanStr1() ); 
	else
		val = cubic_interp( q3, index, q4, leftTanStr1(), topTanStr1() ); 

	return val;
}
  
double FuselageXSec::get_tan_str2( int index )
{
	int q2 = num_pnts/2;
	int q1 = q2/2;
	int q3 = q2 + q1;
	int q4 = num_pnts-1;

	double val = 0.0;
	if ( index <= q1 )
		val = cubic_interp( 0,  index, q1, topTanStr2(), rightTanStr2() ); 
	else if ( index <= q2 )
		val = cubic_interp( q1, index, q2, rightTanStr2(), botTanStr2() ); 
	else if ( index <= q3 )
		val = cubic_interp( q2, index, q3, botTanStr2(), leftTanStr2() ); 
	else
		val = cubic_interp( q3, index, q4, leftTanStr2(), topTanStr2() ); 

	return val;
}
  
void FuselageXSec::setShapeSymmetry()
{
	if ( topSymFlag )
	{
		botTanAng.set( topTanAng() );
		botTanStr1.set( topTanStr1() );
		botTanStr2.set( topTanStr2() );
	}
	if ( sideSymFlag )
	{
		rightTanAng.set(  leftTanAng() );
		rightTanStr1.set( leftTanStr1() );
		rightTanStr2.set( leftTanStr2() );
	}
}


//===== Compute Point Distribution =====//
void FuselageXSec::compute_pnt_distribution( int& num_top, int& num_circle, int& num_bot )
{
  int num_pnt_half = num_pnts/2 + 1;

  double l_top    = top_crv.get_length();
  double l_bot    = bot_crv.get_length();

  double l_circle = (double)( corner_rad()*height()*(PI - DEG_2_RAD*top_tan_angle() - DEG_2_RAD*bot_tan_angle()) );

  double total_length = l_top + l_bot + l_circle;

  double circle_fract = 0;
  if ( total_length > DBL_EPSILON )
	  circle_fract = l_circle/total_length;

  if ( pntSpaceType == PNT_SPACE_FIXED )
  {
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
	  num_bot = num_pnt_half - num_circle - num_top;
    }
  }
}

double FuselageXSec::computeArea()
{
	vector< vec3d > pntVec;
	for ( int i = 0 ; i < num_pnts ; i++ )
	{
		pntVec.push_back( pnts[i] );
	}

    vec3d zero;
	return poly_area( pntVec, zero );
}

