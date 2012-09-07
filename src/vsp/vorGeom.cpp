//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#include "aircraft.h"
#include "vorGeom.h"
#include "vorviewScreen.h"
#include <FL/fl_message.H>


#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include "direct.h"
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CaseData::CaseData()
{
	int i;

	lax = 0;				// Card2
	hag = 0.0;
	isweep  = 0;
	itrmax = 0;
	idetail = 0;
	spc = 1.0;
	floatx = 0.0;
	floaty = 0.0;

	nmach = 1;				// Card3

	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		xmach[i] = 0.0;

	xmach[0] = 0.5;
	xmach[1] = 0.75;
	xmach[2] = 1.25;

	nalpha = 1;				// Card4
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		alpha[i] = 0.0;
	alpha[0] = 1.0;
	alpha[1] = 2.0;
	alpha[2] = 5.0;

	latral = 1;				// Card5
	psi = 0.0;
	pitchq = 0.0;
	rollq  = 0.0;
	yawq   = 0.0;
	vinf   = 1.0;

	sref  = 7150.0;			// Card 6
	ar    = 2.5;
	taper = 0.4;
	cbar  = 0.0;
	xbar  = 0.0;
	zbar  = 0.0;
	wspan = sqrt( ar*sref );

	nsbpan = 1000;			// Card 7

	ltail  = 15.0;			// Card 8
	ztail  = 5.0;
	swpwng = 0.0;
	istab  = 0;
	iaero  = 0;
	cdmach = 0.0;
	cdo    = 0.0;

	iburst = 0;				// Card 9
	xref   = 0.0;
	angcrt = 0.0;
	ck1 = 1.0;
	ck2 = 0.0;

	iac = 0;				// Card 10
	aceps = 0.0001;

	itrim = 0;				// Card 11
	ntrim = 1;

	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
	{
		alpha_trim[i] = 0.0;
		cltrim[i] = 0.0;
		cmtrim[i] = 0.0;
	}
	cmeps = 0.001;

	reinf  = 0.0;			// Card 12
	reflen = 1.0;
	idrag  = 0;
	numren = 0;
	nummch = 0;


}

CaseData::~CaseData()
{


}

int CaseData::read_file( const char* file_name )
{
	int i;
	FILE *fp;
	vector< double > tVec;

	vector< Stringc > wVec;
	char str[256];

	//==== Make Sure File Exists ====//
	if ( (fp = fopen(file_name, "r")) == (FILE *)NULL )
	{
		return 0;
	}

	while ( fscanf( fp, "%s", str ) != EOF )
	{
		Stringc sc(str);
		sc.change_from_to( ',', ' ' );
		sc.change_from_to( '=', ' ' );

		int wordcnt = sc.count_words();

		for ( int w = 0 ; w < wordcnt ; w++ )
		{
			wVec.push_back( sc.get_word(w) );
		}
	}

	fclose( fp );

	//==== Read In Values From .cas Parse ====//
	lax = findInt( "LAX", wVec, lax );				// Card2
	hag = findDouble( "HAG", wVec, hag );
	isweep = findInt( "ISWEEP", wVec, isweep );	
	itrmax = findInt( "ITRMAX", wVec, itrmax );	
	idetail = findInt( "IDETAIL", wVec, idetail );	
	spc = findDouble( "SPC", wVec, spc );
	floatx = findDouble( "FLOATX", wVec, floatx );
	floaty = findDouble( "FLOATY", wVec, floaty );

	nmach = findInt( "NMACH", wVec, nmach );		// Card3	
	vector<double> dv;
	dv = findDoubleVec( "XMACH", wVec, dv );
	for ( i = 0 ; i < (int)dv.size() ; i++ )
		xmach[i] = dv[i];

	nalpha = findInt( "NALPHA", wVec, nalpha );		// Card4

	vector<double> adv;
	adv = findDoubleVec( "ALPHA", wVec, adv );
	for ( i = 0 ; i < (int)dv.size() ; i++ )
		alpha[i] = adv[i];


	latral = findInt( "LATRAL", wVec, latral );		// Card5
	psi    = findDouble( "PSI", wVec, psi );
	pitchq = findDouble( "PITCHQ", wVec, pitchq );
	rollq  = findDouble( "ROLLQ", wVec, rollq );
	yawq   = findDouble( "YAWQ", wVec, yawq );
	vinf   = findDouble( "VINF", wVec, vinf );

	sref   = findDouble( "SREF", wVec, sref );		// Card 6
	ar     = findDouble( "AR", wVec, ar );
	taper  = findDouble( "TAPER", wVec, taper );
	cbar   = findDouble( "CBAR", wVec, cbar );
	xbar   = findDouble( "XBAR", wVec, xbar );
	zbar   = findDouble( "ZBAR", wVec, zbar );
	wspan  = findDouble( "WSPAN", wVec, wspan );

	nsbpan = findInt( "NSBPAN", wVec, nsbpan );		// Card7
	
	ltail    = findDouble( "LTAIL", wVec, ltail );	// Card 8
	ztail    = findDouble( "ZTAIL", wVec, ztail );		
	swpwng   = findDouble( "SWPWNG", wVec, swpwng );	
	istab    = findInt( "ISTAB", wVec, istab );	
	iaero    = findInt( "IAERO", wVec, iaero );	
	cdmach   = findDouble( "CDMACH", wVec, cdmach );	
	cdo      = findDouble( "CDO", wVec, cdo );	

	iburst = findInt( "IBURST", wVec, iburst );		// Card 9
	xref   = findDouble( "XREF", wVec, xref );		
	angcrt = findDouble( "ANGCRT", wVec, angcrt );		
	ck1    = findDouble( "CK1", wVec, ck1 );		
	ck2    = findDouble( "CK2", wVec, ck2 );		

	iac     = findInt( "IAC", wVec, iac );			// Card 10
	aceps   = findDouble( "ACEPS", wVec, aceps );		

	itrim   = findInt( "ITRIM", wVec, itrim );		// Card 11
	ntrim   = findInt( "NTRIM", wVec, ntrim );	

	tVec = findDoubleVec( "ALPHA_TRIM", wVec, tVec );
	for ( i = 0 ; i < (int)tVec.size() ; i++ )
		alpha_trim[i] = tVec[i];

	tVec = findDoubleVec( "CLTRIM", wVec, tVec );
	for ( i = 0 ; i < (int)tVec.size() ; i++ )
		cltrim[i] = tVec[i];

	tVec = findDoubleVec( "CMTRIM", wVec, tVec );
	for ( i = 0 ; i < (int)tVec.size() ; i++ )
		cmtrim[i] = tVec[i];

	cmeps        = findDouble( "CMEPS", wVec, cmeps );		

	
	reinf    = findDouble( "REINF", wVec, reinf );	// Card 12
	reflen   = findDouble( "REFLEN", wVec, reflen );		
	idrag    = findInt( "IDRAG", wVec, idrag );		
	numren    = findInt( "NUMREN", wVec, numren );		
	nummch    = findInt( "NUMMCH", wVec, nummch );		

	return 1;

}

int CaseData::write_file( const char* file_name )
{
	int i;
	FILE *fp;

	//==== Make Sure File Exists ====//
	if ( (fp = fopen(file_name, "w")) == (FILE *)NULL )
	{
		return 0;
	}

	//==== Card 2 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD2 LAX = %d, HAG = %.3f, ISWEEP = %d, ITRMAX = %d,\n", lax, hag, isweep, itrmax);
	fprintf(fp, "  IDETAIL = %d, SPC = %.3f, FLOATX = %.3f, FLOATY = %.3f, $END\n", idetail, spc, floatx, floaty );

	//==== Card 3 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD3 NMACH = %d, \n", nmach);
	fprintf(fp, "  XMACH = %.3f,",  xmach[0] );

	int numm = 20;
	if ( nmach > numm ) numm = nmach;

	for ( i = 1 ; i < numm ; i++ )
	{
		fprintf(fp, "  %.3f,",  xmach[i] );
		if ( i%8 == 0 )
			fprintf(fp, "\n");
	}
	fprintf(fp, "  $END \n" );

	//==== Card 4 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD4 NALPHA = %d, \n", nalpha);
	fprintf(fp, "  ALPHA = %.3f,",  alpha[0] );

	int numa = 20;
	if ( nalpha > numa ) numa = nalpha;

	for ( i = 1 ; i < numa ; i++ )
	{
		fprintf(fp, "  %.3f,",  alpha[i] );
		if ( i%8 == 0 )
			fprintf(fp, "\n");
	}
	fprintf(fp, "  $END \n" );

	//==== Card 5 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD5 LATRAL = %d, PSI =  %.3f, PITCHQ =  %.3f, \n", latral, psi, pitchq);
	fprintf(fp, "        ROLLQ = %.3f, YAWQ =  %.3f, VINF =  %.3f, ", rollq, yawq, vinf);
	fprintf(fp, "  $END \n" );

	//==== Card 6 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD6 SREF = %.3f, AR =  %.3f, TAPER =  %.3f, \n", sref, ar, taper);
	fprintf(fp, "        CBAR = %.3f, XBAR =  %.3f, ZBAR =  %.3f, WSPAN = %.3f, ", cbar, xbar, zbar, wspan);
	fprintf(fp, "  $END \n" );

	//==== Card 7 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD7 NSBPAN = %d, ", nsbpan);
	fprintf(fp, "  $END \n" );

	//==== Card 8 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD8 LTAIL = %.3f, ZTAIL = %.3f, SWPWNG = %.3f, \n", ltail, ztail, swpwng );
	fprintf(fp, "        ISTAB = %d, IAERO = %d, CDMACH = %.3f, CDO = %.3f, ", istab, iaero, cdmach, cdo);
	fprintf(fp, "  $END \n" );

	//==== Card 9 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD9 IBURST = %d, XREF = %.3f, ANGCRT = %.3f, \n", iburst, xref, angcrt );
	fprintf(fp, "        CK1 = %.3f, CK2 =  %.3f, ", ck1, ck2 );
	fprintf(fp, "  $END \n" );

	//==== Card 10 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD10 IAC = %d, ACEPS = %.3f, ", iac, aceps );
	fprintf(fp, "  $END \n" );

	//==== Card 11 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD11 ITRIM = %d, NTRIM = %d, \n", itrim, ntrim );

	numa = ntrim;

	fprintf(fp, "  ALPHA_TRIM = %.3f,",  alpha_trim[0] );
	for ( i = 1 ; i < numa ; i++ )
	{
		fprintf(fp, "  %.3f,",  alpha_trim[i] );
		if ( i%8 == 0 )
			fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "  CLTRIM = %.3f,",  cltrim[0] );
	for ( i = 1 ; i < numa ; i++ )
	{
		fprintf(fp, "  %.3f,",  cltrim[i] );
		if ( i%8 == 0 )
			fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "  CMTRIM = %.3f,",  cmtrim[0] );
	for ( i = 1 ; i < numa ; i++ )
	{
		fprintf(fp, "  %.3f,",  cmtrim[i] );
		if ( i%8 == 0 )
			fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "        CMEPS = %.3f, ", cmeps );
	fprintf(fp, "  $END \n" );

	//==== Card 12 ====//
	fprintf(fp, "\n");
	fprintf(fp, " $CARD12 REINF = %.3f, REFLEN = %.3f, \n", reinf, reflen);
	fprintf(fp, "        IDRAG = %d, NUMREN = %d, NUMMCH = %d,", idrag, numren, nummch);
	fprintf(fp, "  $END \n" );

	fclose( fp );
	return 1;
}

double CaseData::findDouble( const char* name, vector< Stringc > & wVec, double defVal )
{
	for ( int i = 0 ; i < (int)wVec.size()-1 ; i++ )
	{
		if ( wVec[i] == name )
			return atof(wVec[i+1]);
	}
	return defVal;
}

int CaseData::findInt( const char* name, vector< Stringc > & wVec, int defVal )
{
	for ( int i = 0 ; i < (int)wVec.size()-1 ; i++ )
	{
		if ( wVec[i] == name )
			return atoi(wVec[i+1]);
	}
	return defVal;
}

vector<double> CaseData::findDoubleVec( const char* name, vector< Stringc > & wVec, vector<double> defVal )
{
	vector<double> retVec;
	for ( int i = 0 ; i < (int)wVec.size()-1 ; i++ )
	{
		if ( wVec[i] == name )
		{
			i++;
			while ( i < (int)wVec.size() )
			{
				double val;
				if ( wVec[i].is_double( &val ) )
					retVec.push_back( val );
				else
					return retVec;
				i++;		
			}
		}
	}
	return defVal;


}

//====================================================================
//====================================================================
//====================================================================
//====================================================================
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
VorGeom::VorGeom(Aircraft* aptr) : Geom(aptr)
{
	int i;
	type = VOR_GEOM_TYPE;
	type_str = Stringc("hrm");

	validGeomFlag = 0;
	setName( Stringc("VorlaxGeom") );

	vorSlicePtr = new VorSlice();

	baseName = aptr->getName();

	for ( i = 0 ; i < NUM_DRAW_FLAGS ; i++ )
		drawFlags[i] = 0;

	drawFlags[DRAW_XSEC]  = 1;
	drawFlags[DRAW_SLICE] = 1;

	for ( i = 0 ; i < NUM_DRAW_FLAGS ; i++ )
		saveDrawFlags[i] = drawFlags[i];


	setNoShowFlag(1);

	vorviewScreenPtr = 0;

}

VorGeom::~VorGeom()
{
	delete vorSlicePtr;
}

void VorGeom::write(xmlNodePtr root)
{
}

//==== Read External File ====//
void VorGeom::read(xmlNodePtr root)
{
}

//==== Get Clamp Vals ====//
float VorGeom::getCpClamps( int id )
{
	if ( id == CP_MIN )
		return vorSlicePtr->Vor->cp_min;
	else if ( id == CP_MAX )
		return vorSlicePtr->Vor->cp_max;
	else if ( id == CP_MIN_CLAMP )
		return vorSlicePtr->Vor->min_clamp;
	else if ( id == CP_MAX_CLAMP )
		return vorSlicePtr->Vor->max_clamp;
	else if ( id == CP_SURF_MIN )
		return vorSlicePtr->Vor->cp_surf_min;
	else if ( id == CP_SURF_MAX )
		return vorSlicePtr->Vor->cp_surf_max;
	else if ( id == CP_MIN_SURF_CLAMP )
		return vorSlicePtr->Vor->cp_surf_min_clamp;
	else if ( id == CP_MAX_SURF_CLAMP )
		return vorSlicePtr->Vor->cp_surf_max_clamp;

	return 0;
}


void VorGeom::incCpClamps( int minInc, int maxInc )
{
	if ( getDrawFlag(DRAW_CP) )
	{
		vorSlicePtr->findcpcolors(minInc, maxInc);
	}
	else
	{
		vorSlicePtr->findrgbcolors(minInc, maxInc);
	}

}


vec3d VorGeom::getCpColor( double fract )
{
	short cvec[3];
	vorSlicePtr->find_rgb( (float)fract, cvec );

	return vec3d( cvec[0], cvec[1], cvec[2] );
}
	
void VorGeom::setCamberScale( double s )
{
	vorSlicePtr->setcamberzoom((float)s);
}


void VorGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != VOR_GEOM_TYPE )
		return;

	compose_model_matrix();
}


//==== Parm Has Changed ReGenerate Fuse Component ====//
void VorGeom::parm_changed(Parm* p)
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

			compose_model_matrix();	
		}
		break;
	}

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}


void VorGeom::acceptScaleFactor()
{
}

void VorGeom::resetScaleFactor()
{
}

//==== Generate Cross Sections =====//
void VorGeom::update_bbox()
{
  int c, i, j;
  vec3d tmp_pnt;
  bbox new_box;

  for ( c = 0 ; c < (int)compVec.size() ; c++ )
  {
	for ( i = 0 ; i < compVec[c].numCross ; i++)
	{
		for ( j = 0 ; j < compVec[c].numPnts ; j++)
		{
		  new_box.update( compVec[c].crossVec[i][j] );
		}
	}
  }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox

}

void VorGeom::displayControl( int flag )
{
	int i;
	if ( flag )
	{
		for ( i = 0 ; i < NUM_DRAW_FLAGS ; i++ )
		{
			saveDrawFlags[i] = drawFlags[i];
			drawFlags[i] = 0;
		}	
		drawFlags[DRAW_CSF] = 1;
		drawFlags[DRAW_SLICE] = 1;
	}
	else
	{
		for ( i = 0 ; i < NUM_DRAW_FLAGS ; i++ )
		{
			drawFlags[i] = saveDrawFlags[i];
		}
		drawFlags[DRAW_CSF] = 0;
	}
}


void VorGeom::draw()
{
	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
	glLineWidth( 1.0 );

	//==== Draw Geom ====//
	glPushMatrix();
	glMultMatrixf((GLfloat*)model_mat); 

	if ( drawFlags[DRAW_XSEC] )
	{
		if ( airPtr->getGeomVec().size() == 0 )
			vorSlicePtr->drawherm();
	}

	if ( drawFlags[DRAW_SLICE] )
	{
		glColor3ub( 0, 0, 0 );	
		vorSlicePtr->drawslices();
	}
	if ( drawFlags[DRAW_CSF] )
	{
		vorSlicePtr->drawcsf();
	}

	if ( drawFlags[DRAW_SUBPOLY] )
	{
		glColor3ub( 0, 0, 0 );	
		vorSlicePtr->drawsubpols();
	}

	if ( drawFlags[DRAW_CP] )
	{
		vorSlicePtr->drawcpvals();
	}

	if ( drawFlags[DRAW_CP_SURF] )
	{
		vorSlicePtr->drawrgb();
	}

	if ( drawFlags[DRAW_CAMBER] )
	{
		glColor3ub( 255, 255, 0 );	
		glLineWidth( 2.0 );
		vorSlicePtr->drawcamber();
	}


	glPopMatrix();
}

//==== Draw If Alpha < 1 and Shaded ====//
void VorGeom::drawAlpha()
{
}

void VorGeom::readVorlaxFiles()
{
	vorSlicePtr->clear();

	int ierr = vorSlicePtr->readherm( baseName.get_char_star() );

	ierr = vorSlicePtr->readslice( baseName.get_char_star() );

	ierr = vorSlicePtr->read_key_slice( baseName.get_char_star() );

	Stringc cf = baseName;
	cf.concatenate(".cas");
	ierr = caseData.read_file( cf.get_char_star() );

	Stringc scf = baseName;
	scf.concatenate(".srf");
	ierr = vorSlicePtr->calc_surf_press( scf.get_char_star() );

	ierr = vorSlicePtr->readcsf( baseName.get_char_star() );


	/*--- Read subdivided trapizoid data ----*/
 	ierr = vorSlicePtr->readsubpol( baseName.get_char_star() );
    if (!ierr)
    {
		caseData.nsbpan = vorSlicePtr->Vor->nsubp;

		if (!vorSlicePtr->readcpvals(baseName.get_char_star()))
		/*--- Read cp values ----*/
		{
			vorSlicePtr->minmax_cp();
			vorSlicePtr->initclamps();
			vorSlicePtr->findcpcolors(0,0);
		}
    }

}

void VorGeom::readInputVorlaxFiles()
{
	vorSlicePtr->clear();

	int ierr = vorSlicePtr->readherm( baseName.get_char_star() );

	ierr = vorSlicePtr->readslice( baseName.get_char_star() );

	ierr = vorSlicePtr->read_key_slice( baseName.get_char_star() );

}

int VorGeom::read_xsec( const char* file_name )
{
	//==== Extract Base Name ====//
	baseName = Stringc( file_name );
	baseName.remove_substring(".hrm", 1);

	int i, j, c;
	FILE *fp;
	char str[256];

	//==== Make Sure File Exists ====//
	if ( (fp = fopen(file_name, "r")) == (FILE *)NULL )
	{
		return 0;
	}

    //==== Read first Line of file and compare against expected header ====//
    fscanf(fp,"%s INPUT FILE\n\n",str);
	if ( strcmp("HERMITE",str) != 0 )  
	{
		return 0;
	}
     //==== Read in number of components ====//
	int num_comps;
	fscanf(fp," NUMBER OF COMPONENTS = %d\n",&num_comps);

	if ( num_comps <= 0 )
		return 0;

	compVec.resize(num_comps);

	for ( c = 0 ; c < num_comps ; c++ )
	{
		char name_str[256];
		float x, y, z;
		int group_num, type;
		int num_pnts, num_cross;

		fgets( name_str, 256, fp );
//		fscanf(fp,"%s\n",name_str);
		fscanf(fp," GROUP NUMBER = %d\n",&group_num);
		fscanf(fp," TYPE = %d\n",&type);
		fscanf(fp," CROSS SECTIONS = %d\n",&(num_cross));
		fscanf(fp," PTS/CROSS SECTION = %d\n",&(num_pnts));

		//==== Set Component Values ====//
		compVec[c].name_str = name_str;
		compVec[c].groupNum = group_num;
		compVec[c].type     = type;
		compVec[c].numCross = num_cross;
		compVec[c].numPnts  = num_pnts;

		//===== Size Cross Vec ====//
		compVec[c].crossVec.resize( num_cross );
		for ( i = 0 ; i < num_cross ; i++)
		{
			compVec[c].crossVec[i].resize( num_pnts );
			for ( j = 0 ; j < num_pnts ; j++)
			{
				fscanf( fp, "%f %f %f\n", &x, &y, &z );
				compVec[c].crossVec[i][j] = vec3d(x,y,z);
			}
		}
	}
	fclose(fp);

	update_bbox();

	readVorlaxFiles();

	if ( compVec.size() )
	{
		validGeomFlag = 1;
	}


	return 1;
}

void VorGeom::convertToHerm()
{
	//==== Convert Geom To Hermite ====//
	validGeomFlag = 0;
	if ( airPtr->getGeomVec().size() >= 1 )
	{
		Stringc fn = getBaseName();
		fn.concatenate( Stringc(".hrm") );
		airPtr->write_xsec_file( fn.get_char_star() );
		if ( read_xsec( fn ) )
			validGeomFlag = 1;
	}
	else if ( compVec.size() )
	{
		validGeomFlag = 1;
	}
}

void VorGeom::autoFind()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->auto_find();

	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}

void VorGeom::autoEliminate()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->auto_eliminate();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}

void VorGeom::autoMerge()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->auto_merge();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}

void VorGeom::autoSlice()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->auto_slice();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}

void VorGeom::autoAll()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->auto_find();
	vorSlicePtr->auto_eliminate();
	vorSlicePtr->auto_merge();
	vorSlicePtr->auto_slice();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}

void VorGeom::autoReset()
{
	convertToHerm();

	if ( !validGeomFlag )
		return;

	vorSlicePtr->reset_key_slice();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}


int  VorGeom::runVorlax(int type)
{
	int i;
	int numsub, icalc, isurf;
	int swap_xy = 0;
	int stab    = 0;
	double delta;

	if ( vorviewScreenPtr )
		vorviewScreenPtr->updateVorlaxProgress( 0.10 );

	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());


	//if ( vorSlicePtr->Vor->ntraps == 0 )
	//{
	//	fl_alert("No Slices");
	//	return 0;
	//}

	if ((vorSlicePtr->Vor->ntotcsf > 0) && (vorSlicePtr->Vor->csfcheck == vorSlicePtr->Vor->ntraps))
	{
		vorSlicePtr->write_csf_trap( baseName.get_char_star() );
		numsub = 0;
		delta = 0.;
		icalc = 1;
		isurf = 0;


		forkVorlax(type, isurf, icalc, delta, stab);
		waitVorlax();
		readVorlaxOut( &isurf, &icalc, &delta, &stab );
//		numsub = run_vorlax(argv[1],argv[1],&isurf,&icalc,numsub,&delta,&stab);
		if ( vorviewScreenPtr )
			vorviewScreenPtr->updateVorlaxProgress( 0.30 );

		//==== Run control derivative cases ====//
		if ( stab == 1 ) 
		{
			for (i = 0 ; i < vorSlicePtr->Vor->ntotcsf ; i++) 
			{        
				if ( swap_xy == 0 ) 
				{
					vorSlicePtr->Vor->csfdefangle[i] += 1.;
				}
				else 
				{
					vorSlicePtr->Vor->csfdefangle[i] -= 1.;
				}

				vorSlicePtr->write_csf_trap( baseName.get_char_star() );
				delta = vorSlicePtr->Vor->csfdefangle[i];
				icalc = i + 2;
				isurf = i + 1;

				forkVorlax(type, isurf, icalc, delta, stab);
				waitVorlax();
				readVorlaxOut( &isurf, &icalc, &delta, &stab );
//				run_vorlax(argv[1],Vor->csfname[i],&isurf,&icalc,numsub,&delta,&stab);

				if ( swap_xy == 0 ) 
				{
					vorSlicePtr->Vor->csfdefangle[i] -= 1.;
				}
				else 
				{
					vorSlicePtr->Vor->csfdefangle[i] += 1.;
				}
			}
		}

		else if ( stab == 2 ) 
		{
			while ( stab ) 
			{
				i = 0;
				if ( icalc == 1 ) 
					icalc = 2;
				else 
					icalc = 1;

				if ( icalc == 2 ) 
				{
					if ( swap_xy == 0 ) 
						vorSlicePtr->Vor->csfdefangle[i] += 1.;
					else 
						vorSlicePtr->Vor->csfdefangle[i] -= 1.;
				}

				vorSlicePtr->write_csf_trap( baseName.get_char_star() );
				delta = vorSlicePtr->Vor->csfdefangle[i];
				isurf = i + 1;

				forkVorlax(type, isurf, icalc, delta, stab);
				waitVorlax();
				readVorlaxOut( &isurf, &icalc, &delta, &stab );
//				run_vorlax(argv[1],Vor->csfname[i],&isurf,&icalc,numsub,&delta,&stab);

				if ( icalc == 2 ) 
				{
					if ( swap_xy == 0 ) 
						vorSlicePtr->Vor->csfdefangle[i] -= 1.;
					else 
						vorSlicePtr->Vor->csfdefangle[i] += 1.;

					vorSlicePtr->Vor->csfdefangle[i] += (float)delta;   
				}                       
			}
		}
                                    
//		clsstb_();
		vorSlicePtr->write_csf_trap( baseName.get_char_star() );
	}
	else
	{
		numsub = 0;
		delta = 0.;
		icalc = 1;
		isurf = 0;

		forkVorlax(type, isurf, icalc, delta, stab);
		waitVorlax();
		readVorlaxOut( &isurf, &icalc, &delta, &stab );

//		numsub = run_vorlax(argv[1],argv[1],&isurf,&icalc,numsub,&delta,&stab);
//		clsstb_();
	}

	if ( vorviewScreenPtr )
		vorviewScreenPtr->updateVorlaxProgress( 1.00 );


	return 0;
}



int VorGeom::forkVorlax(int type, int isurf, int icalc, double delta, int stab )
{
	if ( !validGeomFlag )
		return 0;

#ifdef WIN32
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

#endif

	//char dir[256];
	//#ifdef WIN32
	//	_getcwd(dir, 255);
	//#else
	//	getcwd( dir, 255 );
	//#endif

	//==== Case File Name ====//
	Stringc  casName = baseName;
	casName.concatenate(".cas");

	//if ( casName.get_length() > 79 )
	//{
	//	fl_alert("File Name Over 80 Characters\n");
	//}

	//==== Write Out Case File =====//
	caseData.write_file( casName );

	//==== Title Name ====//
	Stringc full_path_name = baseName;
	full_path_name.change_from_to('\\', ' ');
	full_path_name.change_from_to('/', ' ');
	Stringc titleName = full_path_name.get_word( full_path_name.count_words()-1 );

	int surfCpFlag = 0;

	int    ISYSCUT = 1;
	int    RSTMCH = 0;
	int    RSTREN = 0;
	double NEWMCH = 0.0;
	double NEWREN = 0.0;

	if ( type == EXECUTE_SURFACE_CP )
	{		
		//==== Surf Press File Name ====//
	    Stringc  srfName = baseName;
	    srfName.concatenate(".srf");

		vorSlicePtr->write_surf_press_pnts(srfName);

		vorSlicePtr->calc_surf_press(srfName);

		surfCpFlag = 1;
		icalc = 1;
	}
	else if ( type == EXECUTE_SUBDIVIDE )
	{
		icalc = 0;
	}
 
	//==== Write Out To Input File ====//
	FILE* fp = fopen("toVorview.txt", "w");
	fprintf(fp,"%s\n", casName.get_char_star() );		// Cas file name
	fprintf(fp,"%s\n", titleName.get_char_star() );		// Base file name
	fprintf(fp,"%d\n", surfCpFlag );					// Perform Surface Cp Flag
	fprintf(fp,"%d\n", icalc );							// ICalc Flag
	fprintf(fp,"%d\n", caseData.nsbpan );				// Number of subpanels
	fprintf(fp,"%d\n", isurf );				
	fprintf(fp,"%f\n", delta );
	fprintf(fp,"%d\n", ISYSCUT );
	fprintf(fp,"%d\n", stab );
	fprintf(fp,"%d\n", RSTMCH );
	fprintf(fp,"%d\n", RSTREN );
	fprintf(fp,"%f\n", NEWMCH );
	fprintf(fp,"%f\n", NEWREN );

	fclose(fp);


	//==== Start the child process Vorlax for Windows ====//
#ifdef WIN32
	if( !CreateProcess( NULL,   // No module name (use command line). 
		TEXT("vorlaxWin"), // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
	) 
	{
		printf( "CreateProcess failed (%d).\n", GetLastError() );
		return 0;
	}
#else
 
	childPid = fork();

	if(childPid == -1)
	{
		printf( "Fork failed (%d).\n", childPid );
		return 0;	
	}
    else if ( childPid == 0 )
	{
        #ifdef __APPLE__
 		execl("./vorlaxMac", "vorlaxMac", (char*)0);
        #else
		execl("./vorlaxLinux", "vorlaxLinux", (char*)0);
        #endif
        
		return 0;
	}

#endif

	return 1;

}

int  VorGeom::checkVorlaxRun( double* percentDone )
{
	int cnt = 50;
	int maxCnt = 100;
	FILE* fp = fopen("progressVorlax.txt","r");
	if (fp)
	{
		fscanf(fp, "%d %d", &cnt, &maxCnt );
		fclose(fp);
	}

	double fdone = 0.0;
	if ( maxCnt )
		fdone = (double)cnt/(double)maxCnt;

	*percentDone = fdone;

	// Close process and thread handles. 
#ifdef WIN32												//Windows
	int ret = WaitForSingleObject( pi.hProcess, 100 );
		
	if ( ret == WAIT_OBJECT_0 )
	{
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return 0;
	}
#else	//LINUX
	int status;
	pid_t retpid = waitpid( childPid, &status, WNOHANG );

	if  (retpid == childPid )
	{
//Close???		
		return 0;
	}


#endif
	
	return 1;
}


void VorGeom::waitVorlax()
{
	int waitFlag = 1;

	while ( waitFlag )
	{
		// Close process and thread handles. 
#ifdef WIN32												//Windows
		int ret = WaitForSingleObject( pi.hProcess, 100 );
			
		if ( ret == WAIT_OBJECT_0 )
		{
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );

			waitFlag = 0;
		}
#else	//LINUX
		int status;
		pid_t retpid = waitpid( childPid, &status, WNOHANG );

		if  (retpid == childPid )
		{
			waitFlag = 0;
		}
		else
		{
			usleep(10000);			// Dont hog the system (microseconds)
		}
#endif

		if ( vorviewScreenPtr )
			vorviewScreenPtr->updateVorlaxProgress( 0.50 );

	}
}

void VorGeom::readVorlaxOut( int* isurf, int* icalc, double* delta, int* stab )
{
	int idum;
	FILE* fp = fopen("fromVorlax.txt","r");
	if (fp)
	{
		fscanf(fp, "%d %d %d %d %f %d %d \n", isurf, icalc, &idum, &idum, delta, &idum, stab );
		fclose(fp);
	}
}

void VorGeom::setFlatFlag(int f)
{
	vorSlicePtr->Vor->flat_flag = f;

    vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
	vorSlicePtr->write_flags(baseName.get_char_star());
}

int  VorGeom::getFlagFlag()
{
	return vorSlicePtr->Vor->flat_flag;
}
	
void VorGeom::setKeySliceSymFlag(int f)
{
	vorSlicePtr->set_symmetry_flag(f);
}
	
int  VorGeom::getKeySliceSymFlag()
{
	return vorSlicePtr->get_symmetry_flag();
}

int  VorGeom::getNumKeySlice()
{
	return vorSlicePtr->get_num_key_slices();
}

void VorGeom::setCurrKeySlice(int ind)
{
	vorSlicePtr->set_current_key_slice(ind);
}

int VorGeom::getCurrKeySlice()
{
	return vorSlicePtr->get_current_key_slice();
}

void VorGeom::delCurrKeySlice()
{
	vorSlicePtr->delete_key_slice();

	saveWriteSlices();

}

float VorGeom::getCurrKeySliceY()
{
	if ( vorSlicePtr->get_num_key_slices() > 0 )
	{
		return vorSlicePtr->get_current_key_slice_y();
	}
	return 0.0;
}

float VorGeom::getCurrKeySliceYMin()
{
	return -100.0;
}
float VorGeom::getCurrKeySliceYMax()
{
	return 100.0;
}

float VorGeom::getCurrKeySliceYFract()
{
	float val = getCurrKeySliceY();

	float ymin = vorSlicePtr->key_slice_ymin();
	float ymax = vorSlicePtr->key_slice_ymax();
	float yspan = ymax - ymin;

	if ( yspan > 0.0000001 )
	{
		return ( (val - ymin)/yspan );
	}
	return 0.0;
}

void VorGeom::setEditKeySliceYFract( float yf, int sym_flag )
{
	float ymin = vorSlicePtr->key_slice_ymin();
	float ymax = vorSlicePtr->key_slice_ymax();

	float y = ymin + yf*(ymax - ymin);


	int id = vorSlicePtr->get_current_key_slice();

	vorSlicePtr->set_current_key_slice_y( id, y );

	if ( sym_flag )
	{
		int sym_id = vorSlicePtr->get_num_key_slices() - id - 1;

		float syf = 1.0f - yf;
		float sy = ymin + syf*(ymax - ymin);
		vorSlicePtr->set_current_key_slice_y( sym_id, sy );
	}

	saveWriteSlices();
}

void VorGeom::saveWriteSlices()
{
	int curr_key_id = vorSlicePtr->get_current_key_slice();

	vorSlicePtr->write_key_slice( baseName.get_char_star() );
	vorSlicePtr->auto_slice();
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());

	setCurrKeySlice( curr_key_id );
}

void VorGeom::setCurrNumRegionSlices( int n )
{
	vorSlicePtr->set_current_nslices( n );
	vorSlicePtr->compute_slice();
	vorSlicePtr->write_trap(baseName.get_char_star());
	vorSlicePtr->write_key_slice(baseName.get_char_star());
	vorSlicePtr->write_key_point(baseName.get_char_star());
}


void  VorGeom::setKeySliceY( float y )
{
	if ( vorSlicePtr->get_num_key_slices() > 0 )
	{
		vorSlicePtr->set_temp_key_slice(y);
//		vorSlicePtr->set_temp_key_slice_y(y);
	}
}

void  VorGeom::adjustKeySliceY( float dir )
{
	vorSlicePtr->adjust_temp_key_slice_y( dir );
}

float VorGeom::getTempKeySliceY()
{
	return vorSlicePtr->get_temp_key_slice_y();
}

void  VorGeom::setKeySliceTempFlag( int flag )
{
	vorSlicePtr->set_key_slice_temp_flag(flag);
}

int   VorGeom::getKeySliceTempFlag()
{
	return vorSlicePtr->get_key_slice_temp_flag();

}

void  VorGeom::addKeySlice()
{
	vorSlicePtr->add_key_slice();

	saveWriteSlices();

}

void VorGeom::writeCsf()
{

	vorSlicePtr->flap_write( baseName.get_char_star() );

}

void VorGeom::setCsfName( const char* val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		strcpy(vorSlicePtr->Vor->csfname[curr_csf], val);
	}
}

void VorGeom::setCsfAngle( double val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csfdefangle[curr_csf] = (float)val;
	}
}

void VorGeom::setCsfChord( double val )
{
	if ( val < 0 )
		val = 0;
	if ( val > 1.0 )
		val = 1.0;

	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csfchord[curr_csf] = (float)val;
	}
}

void VorGeom::setCsfHinge( double val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csfhingepercent[curr_csf] = (float)val;
	}
}

void VorGeom::setCsfType( int val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csftype[curr_csf] = val;
	}
}

void VorGeom::setCsfRefl( int val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csfreflect[curr_csf] = val;
	}
}

void VorGeom::setCsfSym( int val )
{
	int curr_csf = vorSlicePtr->Vor->curr_csf;
		
	if ( curr_csf >= 0 && curr_csf < vorSlicePtr->Vor->ntotcsf )
	{
		vorSlicePtr->Vor->csfsymmetry[curr_csf] = val;
	}
}

bool VorGeom::vorlaxExeExists()
{
#ifdef WIN32
	FILE* fp = fopen("vorlaxWin.exe","r");
#else
	#ifdef __APPLE__
	FILE* fp = fopen("./vorlaxMac","r");
	#else
	FILE* fp = fopen("./vorlaxLinux","r");
	#endif
#endif

	if ( fp )
	{
		fclose(fp);
		return true;
	}
	return false;
}
