//    
//   Havoc Geometry Class
//  
// 
//   J.R. Gloudemans - 11/16/94
//   ETP Software
//
//

#include "havoc_geom.h"
#include "aircraft.h"
#include "materialMgr.h"
#include "parmLinkMgr.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "defines.h"

static double infdenom = 0.0;
static double infTest = 1.0/infdenom;
static double nanTest = sqrt( -1.0 );

//==== Constructor =====//
Havoc_geom::Havoc_geom(Aircraft* aptr) : Geom(aptr)
{
  type = HAVOC_GEOM_TYPE;
	type_str = Stringc("havoc");

  char name[255];
  sprintf( name, "Havoc_%d", geomCnt ); 
  geomCnt++;
  name_str = Stringc(name);
  id_str = name_str;				//jrg fix
  setSymCode(NO_SYM);

  numPnts   = 41;		
  numXsecs  = 41;
     
  numPnts = 4*(numPnts.iget()/4)+1;
  havoc_num_pnts  = numPnts.iget();
  havoc_num_xsecs  = numXsecs.iget();

  define_parms();
  generate();

  surfVec.push_back( &body_surf );


}

//==== Destructor =====//
Havoc_geom::~Havoc_geom()
{
}

void Havoc_geom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != HAVOC_GEOM_TYPE )
		return;

	Havoc_geom* g = (Havoc_geom*)fromGeom;

	length.set( g->get_length()->get() );
	lpiovl.set( g->get_lpiovl()->get() );
	apm.set( g->get_apm()->get() );
	apn.set( g->get_apn()->get() );

	pera.set( g->get_pera()->get() );
	pln.set( g->get_pln()->get() );
	ple.set( g->get_ple()->get() );

	ar.set( g->get_ar()->get() );
	al.set( g->get_al()->get() );

	aum.set( g->get_aum()->get() );
	aun.set( g->get_aun()->get() );
	bu.set( g->get_bu()->get() );

	alm.set( g->get_alm()->get() );
	aln.set( g->get_aln()->get() );
	bl.set( g->get_bl()->get() );

	gum.set( g->get_gum()->get() );
	theta.set( g->get_theta()->get() );
	ptas.set( g->get_ptas()->get() );

	bue.set( g->get_bue()->get() );
	ble.set( g->get_ble()->get() );

	mu.set( g->get_mu()->get() );
	nu.set( g->get_nu()->get() );

	ml.set( g->get_ml()->get() );
	nl.set( g->get_nl()->get() );

	mexp1.set( g->get_mexp1()->get() );
	nexp1.set( g->get_nexp1()->get() );

	mexp2.set( g->get_mexp2()->get() );
	nexp2.set( g->get_nexp2()->get() );

	mexp3.set( g->get_mexp3()->get() );
	nexp3.set( g->get_nexp3()->get() );

	mexp4.set( g->get_mexp4()->get() );
	nexp4.set( g->get_nexp4()->get() );

	numPnts = 4*(numPnts.iget()/4)+1;
	havoc_num_pnts  = numPnts.iget();
	havoc_num_xsecs  = numXsecs.iget();

	body_surf.set_num_pnts(   havoc_num_pnts );		//jrg fix
	body_surf.set_num_xsecs(  havoc_num_xsecs+1 );

	compose_model_matrix();
	generate();



}


//==== Define  Parms ====//
void Havoc_geom::define_parms()
{
  length.initialize(this, UPD_HAVOC, "Length", 25.0);
  length.set_lower_upper(0.0001, 1000000.0);
  length.set_script("havoc_plan length", 0);

  lpiovl.initialize(this, UPD_HAVOC, "Planform Break Point (Lpiovl)", 0.5);
  lpiovl.set_lower_upper(0.01, 0.99);
  lpiovl.set_script("havoc_plan lpiovl", 0);
  
  apm.initialize(this, UPD_HAVOC, "Planform M Exponent (Apm)", 1.7);
  apm.set_lower_upper(0.01, 10.0);
  apm.set_script("havoc_plan apm", 0);
  apn.initialize(this, UPD_HAVOC, "Planform N Exponent (Apn)", 2.3);
  apn.set_lower_upper(0.01, 10.0);
  apn.set_script("havoc_plan apn", 0);

  ar.initialize(this, UPD_HAVOC, "Width Right (Ar)", 0.25);	// Fraction of 1.0
  ar.set_lower_upper(0.01, 1.0);
  ar.set_script("havoc_plan ar", 0);
  al.initialize(this, UPD_HAVOC, "Width Left (Al)", 0.25);	// Fraction of 1.0
  al.set_lower_upper(0.01, 1.0);
  al.set_script("havoc_plan al", 0);

  pera.initialize(this, UPD_HAVOC, "Rear Width Fraction (Pera)", 0.5);
  pera.set_lower_upper(0.0, 2.0);
  pera.set_script("havoc_plan pera", 0);

  pln.initialize(this, UPD_HAVOC, "Nose Length Fraction (Pln)", 0.4);
  pln.set_lower_upper(0.01, 0.99);
  pln.set_script("havoc_side pln", 0);

  ple.initialize(this, UPD_HAVOC, "Engine Length Fraction (Ple)", 0.2);
  ple.set_lower_upper(0.01, 0.99);
  ple.set_script("havoc_side ple", 0);
  
  aum.initialize(this, UPD_HAVOC, "Side M Upper Exponent (Aum)", 3.0);
  aum.set_lower_upper(0.01, 10.0);
  aum.set_script("havoc_side um", 0);
  aun.initialize(this, UPD_HAVOC, "Side N Upper Exponent (Aun)", 1.0);
  aun.set_lower_upper(0.01, 10.0);
  aun.set_script("havoc_side un", 0);
  
  bu.initialize(this, UPD_HAVOC, "Height Lower (Bu)", 0.2);
  bu.set_lower_upper(0.01, 1.0);
  bu.set_script("havoc_side bu", 0);

  alm.initialize(this, UPD_HAVOC, "Side M Lower Exponent (Alm)", 1.7);
  alm.set_lower_upper(0.01, 10.0);
  alm.set_script("havoc_side lm", 0);
  aln.initialize(this, UPD_HAVOC, "Side N Lower Exponent (Aln)", 1.3);
  aln.set_lower_upper(0.01, 10.0);
  aln.set_script("havoc_side ln", 0);
  
  bl.initialize(this, UPD_HAVOC, "Height Lower (Bl)", 0.1);
  bl.set_lower_upper(0.01, 1.0);
  bl.set_script("havoc_side bl", 0);
  
  gum.initialize(this, UPD_HAVOC, "Upper Rear Shape Factor (Gum)", 1.4);
  gum.set_lower_upper(0.01, 10.0);
  gum.set_script("havoc_side gum", 0);
  
  theta.initialize(this, UPD_HAVOC, "Eng Exit Angle (Theta)", 40.5);
  theta.set_lower_upper(0.0, 80.0);
  theta.set_script("havoc_side theta", 0);
  
  ptas.initialize(this, UPD_HAVOC, "Exit Center Height (Ptas)", 0.4);
  ptas.set_lower_upper(0.0, 1.0);
  ptas.set_script("havoc_side ptas", 0);

  bue.initialize(this, UPD_HAVOC, "Exit Height Upper (Bue)", 0.1);
  bue.set_lower_upper(0.0, 1.0);
  bue.set_script("havoc_side bue", 0);

  ble.initialize(this, UPD_HAVOC, "Exit Height Lower (Ble)", 0.1);
  ble.set_lower_upper(0.0, 1.0);
  ble.set_script("havoc_side ble", 0);

  mu.initialize(this, UPD_HAVOC, "Xsec M Upper Exponent (Mu)", 2.0);
  mu.set_lower_upper(0.01, 10.0);
  mu.set_script("havoc_side um", 0);
  nu.initialize(this, UPD_HAVOC, "Xsec N Upper Exponent (Nu)", 2.0);
  nu.set_lower_upper(0.01, 10.0);
  nu.set_script("havoc_side un", 0);

  ml.initialize(this, UPD_HAVOC, "Xsec M Lower Exponent (Ml)", 2.0);
  ml.set_lower_upper(0.01, 10.0);
  ml.set_script("havoc_plan ml", 0);
  nl.initialize(this, UPD_HAVOC, "Xsec N Lower Exponent (Nl)", 2.0);
  nl.set_lower_upper(0.01, 10.0);
  nl.set_script("havoc_side ln", 0);

  mexp1.initialize(this, UPD_HAVOC, "Xsec M Exponent (Mexp1)", 2.0);
  mexp1.set_lower_upper(0.01, 10.0);
  mexp1.set_script("havoc_xsec mexp1", 0);
  nexp1.initialize(this, UPD_HAVOC, "Xsec N Exponent (Nexp1)", 2.0);
  nexp1.set_lower_upper(0.01, 10.0);
  nexp1.set_script("havoc_xsec nexp1", 0);

  mexp2.initialize(this, UPD_HAVOC, "Xsec M Exponent (Mexp2)", 2.0);
  mexp2.set_lower_upper(0.01, 10.0);
  mexp2.set_script("havoc_xsec mexp2", 0);
  nexp2.initialize(this, UPD_HAVOC, "Xsec N Exponent (Nexp2)", 2.0);
  nexp2.set_lower_upper(0.01, 10.0);
  nexp2.set_script("havoc_xsec nexp2", 0);

  mexp3.initialize(this, UPD_HAVOC, "Xsec M Exponent (Mexp3)", 2.0);
  mexp3.set_lower_upper(0.01, 10.0);
  mexp3.set_script("havoc_xsec mexp3", 0);
  nexp3.initialize(this, UPD_HAVOC, "Xsec N Exponent (Nexp3)", 2.0);
  nexp3.set_lower_upper(0.01, 10.0);
  nexp3.set_script("havoc_xsec nexp3", 0);

  mexp4.initialize(this, UPD_HAVOC, "Xsec M Exponent (Mexp4)", 2.0);
  mexp4.set_lower_upper(0.01, 10.0);
  mexp4.set_script("havoc_xsec mexp4", 0);
  nexp4.initialize(this, UPD_HAVOC, "Xsec N Exponent (Nexp4)", 2.0);
  nexp4.set_lower_upper(0.01, 10.0);
  nexp4.set_script("havoc_xsec nexp4", 0);

  numXsecs.set_lower( 9 );							// Enough Xsecs to define Havoc Geom
		
  body_surf.set_num_pnts(   havoc_num_pnts );	
  body_surf.set_num_xsecs(  havoc_num_xsecs+1 );

}

void Havoc_geom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );
	Geom::AddLinkableParm( &lpiovl, parmVec, this, "Design" );
	Geom::AddLinkableParm( &apm, parmVec, this, "Design" );
	Geom::AddLinkableParm( &apn, parmVec, this, "Design" );
	Geom::AddLinkableParm( &pera, parmVec, this, "Design" );
	Geom::AddLinkableParm( &ar, parmVec, this, "Design" );
	Geom::AddLinkableParm( &al, parmVec, this, "Design" );
	Geom::AddLinkableParm( &pln, parmVec, this, "Design" );
	Geom::AddLinkableParm( &ple, parmVec, this, "Design" );
	Geom::AddLinkableParm( &aum, parmVec, this, "Design" );
	Geom::AddLinkableParm( &aun, parmVec, this, "Design" );
	Geom::AddLinkableParm( &bu, parmVec, this, "Design" );
	Geom::AddLinkableParm( &alm, parmVec, this, "Design" );
	Geom::AddLinkableParm( &aln, parmVec, this, "Design" );
	Geom::AddLinkableParm( &bl, parmVec, this, "Design" );
	Geom::AddLinkableParm( &gum, parmVec, this, "Design" );
	Geom::AddLinkableParm( &theta, parmVec, this, "Design" );
	Geom::AddLinkableParm( &ptas, parmVec, this, "Design" );
	Geom::AddLinkableParm( &bue, parmVec, this, "Design" );
	Geom::AddLinkableParm( &ble, parmVec, this, "Design" );
	Geom::AddLinkableParm( &mu, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nu, parmVec, this, "Design" );
	Geom::AddLinkableParm( &ml, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nl, parmVec, this, "Design" );
	Geom::AddLinkableParm( &mexp1, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nexp1, parmVec, this, "Design" );
	Geom::AddLinkableParm( &mexp2, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nexp2, parmVec, this, "Design" );
	Geom::AddLinkableParm( &mexp3, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nexp3, parmVec, this, "Design" );
	Geom::AddLinkableParm( &mexp4, parmVec, this, "Design" );
	Geom::AddLinkableParm( &nexp4, parmVec, this, "Design" );
}


//==== Generate Fuse Component ====//
void Havoc_geom::generate()
{
  generate_planform_curves();
  gen_body_surf();
  update_bbox();
  airPtr->geomMod( this );
}

void Havoc_geom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*length()*scaleFactor() ); 
}


//==== Parm Has Changed ReGenerate Component ====//
void Havoc_geom::parm_changed(Parm* p)
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

		case UPD_HAVOC:
		{
		  //==== Check Parm Constraints ====//
		  if ( lpiovl() > ( pln() + ple() ) )
			ple = lpiovl() - pln(); 
    
		  if ( pln() > lpiovl() )
			pln = lpiovl();

		  generate();
		  updateAttach(0);

		}
		break;

		case UPD_NUM_PNT_XSEC:
		{
			numPnts = 4*(numPnts.iget()/4)+1;
			havoc_num_pnts  = numPnts.iget();
			havoc_num_xsecs  = numXsecs.iget();

			body_surf.set_num_pnts(   havoc_num_pnts );		//jrg fix
			body_surf.set_num_xsecs(  havoc_num_xsecs+1 );

			generate();
		}
		break;

    }

	if ( p == &scaleFactor )	
	{
		scale();
	}

	compose_model_matrix();

}


double Havoc_geom::get_area()
{
  double plan_area = length()*length()*(ar()+al());

  return(plan_area);
}


double Havoc_geom::get_span()
{
  double span = length()*(ar()+al());

  return( span );

}

double Havoc_geom::get_cbar()
{
  return(length());
}

void Havoc_geom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	length.set( length()*current_factor );
    generate();


	lastScaleFactor = scaleFactor();

}

void Havoc_geom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void Havoc_geom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
    scale();
	lastScaleFactor = 1.0;
		
	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}


//==== Generate Planform Curves ====//
void Havoc_geom::generate_planform_curves()
{  
  int i;
  
  //==== Find Xsec Locations ====// 
  int num_xsecs = havoc_num_xsecs - 3;
  array< double > tmp_x;
  tmp_x.init ( num_xsecs );
  
  for ( i = 0 ; i < num_xsecs ; i++ )
    {
      double fract = (double)i/(double)(num_xsecs - 1);
      tmp_x[i] = pow(fract, 1.5);  
    }
  
  int cnt = 0;
  x_locs.init( havoc_num_xsecs );
    
  for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
       x_locs[cnt] = tmp_x[i];
       cnt++;
       if ( lpiovl() > tmp_x[i] && lpiovl() <= tmp_x[i+1] )
         {
           x_locs[cnt] = lpiovl();
           cnt++;         
         }
       if ( pln() > tmp_x[i] && pln() <= tmp_x[i+1] )
         {
           x_locs[cnt] = pln();
           cnt++;         
         }
       if ( (pln()+ple()) > tmp_x[i] && (pln()+ple()) <= tmp_x[i+1] )
         {
           x_locs[cnt] = (pln()+ple());
           cnt++;         
         }    
    }
  x_locs[cnt] = 1.0;


  //==== Allocate Space For Planforms ====//
  left.init( havoc_num_xsecs );
  right.init( havoc_num_xsecs );
  
  //==== Planform ====//
  for ( i = 0 ; i < havoc_num_xsecs ; i++ )
    {
      double x = x_locs[i];
      
      if ( x <= lpiovl() )
        {
          right[i] = ar()*pow(1.0-pow( (1.0-x/lpiovl()), apn()), 1.0/apm());   
          left[i]  = al()*pow(1.0-pow( (1.0-x/lpiovl()), apn()), 1.0/apm());   
        }
      else
        {
          right[i] = ( (x - lpiovl())/(1.0 - lpiovl() ) )*
                     ( pera()*ar() - ar() ) + ar();        
          left[i]  = ( (x - lpiovl())/(1.0 - lpiovl() ) )*
                     ( pera()*al() - al() ) + al();        
        }
      right[i] *= 0.9999f;
      left[i]  *= 0.9999f;
    }


}



//==== Write Havoc File ====//
void Havoc_geom::write(FILE* file_id)
{
/*
  fprintf(file_id, "//================== HAVOC COMPONENT ==================//\n");

  //==== Write General Parms ====// 
  write_general_parms(file_id);

  fprintf(file_id, "%f			Length (ft)\n",length());
  fprintf(file_id, "%f			Lpiovl\n",lpiovl());

  fprintf(file_id, "%f			Apm\n",apm());
  fprintf(file_id, "%f			Apn\n",apn());
  fprintf(file_id, "%f			Pera\n",pera());
  fprintf(file_id, "%f			Pln\n",pln());
  fprintf(file_id, "%f			Ple\n",ple());
  
  fprintf(file_id, "%f			Ar\n",ar());
  fprintf(file_id, "%f			Al\n",al());
  
  fprintf(file_id, "%f			Aum\n",aum());
  fprintf(file_id, "%f			Aun\n",aun());
  fprintf(file_id, "%f			Bu\n",bu());
  
  fprintf(file_id, "%f			Alm\n",alm());
  fprintf(file_id, "%f			Aln\n",aln());
  fprintf(file_id, "%f			Bl\n",bl());
  
  fprintf(file_id, "%f			Gum\n",gum());
  fprintf(file_id, "%f			Theta\n",theta());
  fprintf(file_id, "%f			Ptas\n",ptas());

  fprintf(file_id, "%f			Bue\n",bue());
  fprintf(file_id, "%f			Ble\n",ble());
  
  fprintf(file_id, "%f			Mu\n",mu());
  fprintf(file_id, "%f			Nu\n",nu());
  fprintf(file_id, "%f			Ml\n",ml());
  fprintf(file_id, "%f			Nl\n",nl());

  fprintf(file_id, "%f			Mexp1\n",mexp1());
  fprintf(file_id, "%f			Nexp1\n",nexp1());

  fprintf(file_id, "%f			Mexp2\n",mexp2());
  fprintf(file_id, "%f			Nexp2\n",nexp2());

  fprintf(file_id, "%f			Mexp3\n",mexp3());
  fprintf(file_id, "%f			Nexp3\n",nexp3());

  fprintf(file_id, "%f			Mexp4\n",mexp4());
  fprintf(file_id, "%f			Nexp4\n",nexp4());
*/  
   
}

//==== Write Havoc File ====//
void Havoc_geom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Havoc");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr havoc_node = xmlNewChild( root, NULL, (const xmlChar *)"Havoc_Parms", NULL );

  xmlAddDoubleNode( havoc_node, "Length", length() );
  xmlAddDoubleNode( havoc_node, "Lpiovl", lpiovl() );
  xmlAddDoubleNode( havoc_node, "Apm", apm() );
  xmlAddDoubleNode( havoc_node, "Apn", apn() );
  xmlAddDoubleNode( havoc_node, "Pera", pera() );
  xmlAddDoubleNode( havoc_node, "Pln", pln() );
  xmlAddDoubleNode( havoc_node, "Ple", ple() );

  xmlAddDoubleNode( havoc_node, "Ar", ar() );
  xmlAddDoubleNode( havoc_node, "Al", al() );

  xmlAddDoubleNode( havoc_node, "Aum", aum() );
  xmlAddDoubleNode( havoc_node, "Aun", aun() );
  xmlAddDoubleNode( havoc_node, "Bu", bu() );

  xmlAddDoubleNode( havoc_node, "Alm", alm() );
  xmlAddDoubleNode( havoc_node, "Aln", aln() );
  xmlAddDoubleNode( havoc_node, "Bl", bl() );

  xmlAddDoubleNode( havoc_node, "Gum", gum() );
  xmlAddDoubleNode( havoc_node, "Theta", theta() );
  xmlAddDoubleNode( havoc_node, "Ptas", ptas() );

  xmlAddDoubleNode( havoc_node, "Bue", bue() );
  xmlAddDoubleNode( havoc_node, "Ble", ble() );

  xmlAddDoubleNode( havoc_node, "Mu", mu() );
  xmlAddDoubleNode( havoc_node, "Nu", nu() );
  xmlAddDoubleNode( havoc_node, "Ml", ml() );
  xmlAddDoubleNode( havoc_node, "Nl", nl() );

  xmlAddDoubleNode( havoc_node, "Mexp1", mexp1() );
  xmlAddDoubleNode( havoc_node, "Nexp1", nexp1() );

  xmlAddDoubleNode( havoc_node, "Mexp2", mexp2() );
  xmlAddDoubleNode( havoc_node, "Nexp2", nexp2() );

  xmlAddDoubleNode( havoc_node, "Mexp3", mexp3() );
  xmlAddDoubleNode( havoc_node, "Nexp3", nexp3() );

  xmlAddDoubleNode( havoc_node, "Mexp4", mexp4() );
  xmlAddDoubleNode( havoc_node, "Nexp4", nexp4() );

}

//==== Read Havoc File ====//
void Havoc_geom::read(xmlNodePtr root)
{
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Havoc_Parms", 0 );
  if ( node )
  {
    length = xmlFindDouble( node, "Length", length() );
    lpiovl = xmlFindDouble( node, "Lpiovl", lpiovl() );

    apm = xmlFindDouble( node, "Apm", apm() );
    apn = xmlFindDouble( node, "Apn", apn() );
    pera = xmlFindDouble( node, "Pera", pera() );
    pln = xmlFindDouble( node, "Pln", pln() );
    ple = xmlFindDouble( node, "Ple", ple() );

    ar = xmlFindDouble( node, "Ar", ar() );
    al = xmlFindDouble( node, "Al", al() );
    aum = xmlFindDouble( node, "Aum", aum() );
    aun = xmlFindDouble( node, "Aun", aun() );
    bu = xmlFindDouble( node, "Bu", bu() );

    alm = xmlFindDouble( node, "Alm", alm() );
    aln = xmlFindDouble( node, "Aln", aln() );
    bl = xmlFindDouble( node, "Bl", bl() );

    gum = xmlFindDouble( node, "Gum", gum() );
    theta = xmlFindDouble( node, "Theta", theta() );
    ptas = xmlFindDouble( node, "Ptas", ptas() );

    bue = xmlFindDouble( node, "Bue", bue() );
    ble = xmlFindDouble( node, "Ble", ble() );

    mu = xmlFindDouble( node, "Mu", mu() );
    nu = xmlFindDouble( node, "Nu", nu() );
    ml = xmlFindDouble( node, "Ml", ml() );
    nl = xmlFindDouble( node, "Nl", nl() );

    mexp1 = xmlFindDouble( node, "Mexp1", mexp1() );
    nexp1 = xmlFindDouble( node, "Nexp1", nexp1() );

    mexp2 = xmlFindDouble( node, "Mexp2", mexp2() );
    nexp2 = xmlFindDouble( node, "Nexp2", nexp2() );

    mexp3 = xmlFindDouble( node, "Mexp3", mexp3() );
    nexp3 = xmlFindDouble( node, "Nexp3", nexp3() );

    mexp4 = xmlFindDouble( node, "Mexp4", mexp4() );
    nexp4 = xmlFindDouble( node, "Nexp4", nexp4() );

  }

  generate();


}

//==== Read Fuse File ====//
void Havoc_geom::read(FILE* file_id)
{
  //==== Read General Parms ====// 
  read_general_parms(file_id);

  //==== Read External Parms ====//		
  length.read(file_id);
  lpiovl.read(file_id);
  
  apm.read(file_id);
  apn.read(file_id);
  pera.read(file_id);
  pln.read(file_id);
  ple.read(file_id);

  ar.read(file_id);
  al.read(file_id);
  
  aum.read(file_id);
  aun.read(file_id);
  bu.read(file_id);
  
  alm.read(file_id);
  aln.read(file_id);
  bl.read(file_id);
  
  gum.read(file_id);
  theta.read(file_id);
  ptas.read(file_id);
  
  bue.read(file_id);
  ble.read(file_id);
  
  mu.read(file_id);
  nu.read(file_id);
  
  ml.read(file_id);
  nl.read(file_id);
  
  mexp1.read(file_id);
  nexp1.read(file_id);
  mexp2.read(file_id);
  nexp2.read(file_id);
  mexp3.read(file_id);
  nexp3.read(file_id);
  mexp4.read(file_id);
  nexp4.read(file_id);

  generate();

}

//==== Write Rhino File ====//
void Havoc_geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	body_surf.clear_pnt_tan_flags();
	body_surf.clear_xsec_tan_flags();


	body_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}

int Havoc_geom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void Havoc_geom::write_bezier_file( int id, FILE* file_id )
{
	body_surf.clear_xsec_tan_flags();

	body_surf.set_xsec_tan_flag( 0, Bezier_curve::PREDICT );
    int num_xsecs  = body_surf.get_num_xsecs();
	body_surf.set_xsec_tan_flag( num_xsecs-2, Bezier_curve::SHARP );
	body_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );

	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3*(body_surf.get_num_xsecs() - 1)  );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(body_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(body_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(body_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(body_surf.get_num_pnts() - 1)  );

	body_surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
}


//==== Convert To Tri Mesh ====//
vector< TMesh* > Havoc_geom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

  // Write out final, closed cross section  
//  p = body_surf.get_xsec_center(havoc_num_xsecs-1).transform(model_mat);

	TMesh* tmPtr = body_surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		tmPtr = body_surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}

vec3d Havoc_geom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = body_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}


//==== Generate Cross Sections =====//
void Havoc_geom::regenerate()
{
}

//==== Generate Cross Sections =====//
void Havoc_geom::gen_body_surf()
{
  int i,j;
  vec3d def_pnt, temp_pnt;

  int num_quart = havoc_num_pnts/4 + 1;
  double len = length();

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for (  i = 0 ; i < havoc_num_xsecs ; i++ )
    {  
      double x = x_locs[i];
      double l = left[i];
      double r = right[i];
       
      int pnt_cnt = 0;   
      for (  j = 0 ; j < num_quart ; j++ )
        {
          double fract = (double)j/(double)(num_quart - 1);
          fract = sin( fract*PI/2.0 );
          double z = compute_z( UPPER_RIGHT, x, r*fract );
          temp_pnt.set_xyz( len*x, len*r*fract, len*z );

          body_surf.set_pnt(i, pnt_cnt, temp_pnt);
          pnt_cnt++;
        }

      for (   j = 1 ; j < num_quart ; j++ )
        {
          double fract = (double)(num_quart - 1 - j)/(double)(num_quart - 1);
          fract = sin( fract*PI/2.0 );
          double z = compute_z( LOWER_RIGHT, x, r*fract );
          temp_pnt.set_xyz( len*x, len*r*fract, len*z ); 
          body_surf.set_pnt(i, pnt_cnt, temp_pnt);
          pnt_cnt++;
        }
        
      for (   j = 1 ; j < num_quart ; j++ )
        {
          double fract = (double)j/(double)(num_quart - 1);
          fract = sin( fract*PI/2.0 );
          double z = compute_z( LOWER_LEFT, x, l*fract );
          temp_pnt.set_xyz( len*x, -l*fract*len, len*z ); 
          body_surf.set_pnt(i, pnt_cnt, temp_pnt);
          pnt_cnt++;
        }
        
      for (   j = 1 ; j < num_quart ; j++ )
        {
          double fract = (double)(num_quart - 1 - j)/(double)(num_quart - 1);
          fract = sin( fract*PI/2.0  );
          double z = compute_z( UPPER_LEFT, x, l*fract );
          temp_pnt.set_xyz( len*x, -l*fract*len, len*z );
          body_surf.set_pnt(i, pnt_cnt, temp_pnt);
          pnt_cnt++;
        }
    }
	vec3d p = body_surf.get_xsec_center(havoc_num_xsecs-1);
	for ( j = 0 ; j < havoc_num_pnts ; j++ )
	{
         body_surf.set_pnt(havoc_num_xsecs, j, p);
	}

  body_surf.load_refl_pnts_xsecs();
//jrg  if (main_window_ptr->get_display_type_flag() == DISPLAY_HIDDEN_LINE)
    body_surf.load_hidden_surf();
//jrg  else if (main_window_ptr->get_display_type_flag() == DISPLAY_SHADED_SURF)
    body_surf.load_normals();

   body_surf.load_uw();


}

//==== Update Bounding Box =====//
void Havoc_geom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts = body_surf.get_num_pnts();

  //==== Check Some Inlet And Nozzle Xsecs ====//
  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      pnt = body_surf.get_pnt(0,j);						new_box.update(pnt);
      pnt = body_surf.get_pnt(havoc_num_xsecs/2,j);     new_box.update(pnt);
      pnt = body_surf.get_pnt(havoc_num_xsecs-1,j);     new_box.update(pnt);
    }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}




void Havoc_geom::draw()
{
	Geom::draw();

/*
	if ( fastDrawFlag )
	{
		body_surf.fast_draw_on();
	}
	else
	{
		body_surf.fast_draw_off();
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
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		body_surf.draw_refl_wire(sym_code);
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
				body_surf.draw_shaded();
				if ( displayFlag == GEOM_TEXTURE_FLAG )
					drawTextures(false);
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
		body_surf.draw_hidden();
		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		body_surf.draw_refl_hidden(sym_code);
		glPopMatrix();
			
	}
*/
}



//==== Draw If Alpha < 1 and Shaded ====//
void Havoc_geom::drawAlpha()
{
	Geom::drawAlpha();
/*

	if ( displayFlag == GEOM_SHADE_FLAG || displayFlag == GEOM_TEXTURE_FLAG)
	{	
		Material* mat = matMgrPtr->getMaterial( materialID );
		if ( mat && mat->diff[3] <= 0.99 )
		{
			//==== Draw Geom ====//
			glPushMatrix();
			glMultMatrixf((GLfloat*)model_mat); 

			mat->bind();
			body_surf.draw_shaded();

			if ( displayFlag == GEOM_TEXTURE_FLAG )
				drawTextures(false);

			glPopMatrix();

			//==== Reflected Geom ====//
			glPushMatrix();
			glMultMatrixf((GLfloat*)reflect_mat); 

			body_surf.draw_refl_shaded( sym_code);

			if ( displayFlag == GEOM_TEXTURE_FLAG )
				drawTextures(true);

			glPopMatrix();
		}
	}
*/
}



//==== Compute And Load Normals ====//
void Havoc_geom::load_normals()
{
  body_surf.load_normals();
}

//==== Draw Other Pnts XSecs====//
void Havoc_geom::load_hidden_surf()
{
  body_surf.load_hidden_surf();
}

//==== Compute Z as a Funct====//
double Havoc_geom::compute_z( int quad, double x_fract, double y_fract )
{
  double z;
  if ( x_fract < lpiovl() )
    {
      z = compute_nose_z( quad, x_fract, y_fract );
    }
  else
    {
      z = compute_tail_z( quad, x_fract, y_fract );
    }
  return z;
}

//==== Compute Nose Z ====//
double Havoc_geom::compute_nose_z( int quad, double x, double y )
{
  double z;
  if ( quad == UPPER_RIGHT )
    {
      double tol = 0.00001*ar();
      if ( fabs( x ) < tol )
        z = 0.0;
      else
        {
          double c1 = pow( ( 1.0 - (x/lpiovl())), apn() );
          double c2 = pow( ( 1.0 - c1 ), 1.0/apm() );
          double e3 = ar()*c2;
          double c3 = pow( ( 1.0 - (x/lpiovl())), aun() );
          double c4 = pow( ( 1.0 - c3 ), 1.0/aum() );
          double e4 = bu()*c4;
          double e1 = pow( ( y/e3 ), mexp1() );
          if ( e1 > 1.0 - tol && e1 < 1.0 + tol )
            e1 = 1.0;
          z = e4*pow( (1.0-e1), 1.0/nexp1() );
        }
    }
  else if ( quad == UPPER_LEFT )
    {
      double tol = 0.00001*al();
      if ( fabs( x ) < tol )
        z = 0.0;
      else
        {
          double c1 = pow( ( 1.0 - (x/lpiovl())), apn() );
          double c2 = pow( ( 1.0 - c1 ), 1.0/apm() );
          double e3 = al()*c2;
          double c3 = pow( ( 1.0 - (x/lpiovl())), aun() );
          double c4 = pow( ( 1.0 - c3 ), 1.0/aum() );
          double e4 = bu()*c4;
          double e1 = pow( ( y/e3 ), mexp2() );
          if ( e1 > 1.0 - tol && e1 < 1.0 + tol )
            e1 = 1.0;


          z = e4*pow( (1.0-e1), 1.0/nexp2() );
//					assert( z > -1000 && z < 1000 );
//					assert( z != infTest );
//					assert( z != nanTest );		jrg doesnt seem to work on win
        }
    }
  else if ( quad == LOWER_LEFT )
    {
      if ( x <= 0.0 )
        z = 0.0;
      else
        {  
          double x1 = pln();
          double ts = (pln() + ple() - lpiovl())/( 1.0 - lpiovl() );
          double x3 = (1.0 - ts)*(1.0 - lpiovl());
//          double x2 = 1.0 - x1 - x3;
          
          double c1 = pow( (1.0 - (x/lpiovl())), apn() );
          double c2 = pow( (1.0 - c1), 1.0/apm() );
          double e3 = al()*c2;
          double c3 = pow( (1.0 - (x/lpiovl())), aln() );
          double c4 = pow( (1.0 - c3), 1.0/alm() );
 
          double denom = ( 1.0 - pow( (1.0 - (pln()/lpiovl())), aln()));
          denom = pow( denom, 1.0/alm() );
          double bln = 1.0;
          if ( denom != 0.0 )
            bln = bl()/denom;
          
          double e4 = bl();
          if ( x < x1 )
            e4 = bln*c4;
          
          //==== Dont Know What XFLRS is So Dont Check xxx-jrg ====//
          double e1 = pow( (y/e3), mexp3() ); 
          z = -e4*pow( (1.0-e1), 1.0/nexp3() );

        }
    }
  else if ( quad == LOWER_RIGHT )
    {
      if ( x <= 0.0 )
        z = 0.0;
      else
        {  
          double x1 = pln();
          double ts = (pln() + ple() - lpiovl())/( 1.0 - lpiovl() );
          double x3 = (1.0 - ts)*(1.0 - lpiovl());
//          double x2 = 1.0 - x1 - x3;
          
          double c1 = pow( (1.0 - (x/lpiovl())), apn() );
          double c2 = pow( (1.0 - c1), 1.0/apm() );
          double e3 = ar()*c2;
          double c3 = pow( (1.0 - (x/lpiovl())), aln() );
          double c4 = pow( (1.0 - c3), 1.0/alm() );
 
          double denom = ( 1.0 - pow( (1.0 - (pln()/lpiovl())), aln()));
          denom = pow( denom, 1.0/alm() );
          double bln = 1.0;
          if ( denom != 0.0 )
            bln = bl()/denom;
          
          double e4 = bl();
          if ( x < x1 )
            e4 = bln*c4;
          
          //==== Dont Know What XFLRS is So Dont Check xxx-jrg ====//
          double e1 = pow( (y/e3), mexp4() ); 
          z = -e4*pow( (1.0-e1), 1.0/nexp4() );

        }
    }
           
  return z;    
}

double Havoc_geom::compute_tail_z( int quad, double x, double y )
{
  double z = 0;

  if ( quad == UPPER_RIGHT )
    {
      double ttas = ptas()*bu()/(1.0 - lpiovl());
      
      double c1 = ( x - lpiovl() )/( 1.0 - lpiovl() );
      double c2 = ttas*(1.0 - lpiovl())*c1;
      
      if ( fabs( c1 - 1.0 ) < 0.0001 )
        c1 = 1.0;
        
      double c3 = bue()+(bu() - bue())*pow( (1.0 - c1), 1.0/gum() );
      double tear = ar()*pera();
      double c4 = c1*( tear - ar() ) + ar();
      
      double e1 = y/c4;
      if ( e1 <= 0.0 )
        e1 = 0;
        
      double e1mu = pow( e1, mexp1() );
      
      if ( e1mu > 1.0 )
        e1mu = 1.0;
      
      z = c2 + c3*pow( (1.0 - e1mu), 1.0/nexp1() );
    }
  else if ( quad == UPPER_LEFT )
    {
      double ttas = ptas()*bu()/(1.0 - lpiovl());
      
      double c1 = ( x - lpiovl() )/( 1.0 - lpiovl() );
      double c2 = ttas*(1.0 - lpiovl())*c1;
      
      if ( fabs( c1 - 1.0 ) < 0.0001 )
        c1 = 1.0;

      double c3 = bue()+(bu() - bue())*pow( (1.0 - c1), 1.0/gum() );
      double teal = al()*pera();
      double c4 = c1*( teal - al() ) + al();

      double e1 = y/c4;
      if ( e1 <= 0.0 )
        e1 = 0;
        
      double e1mu = pow( e1, mexp2() );

      if ( e1mu > 1.0 )
        e1mu = 1.0;
      
      z = c2 + c3*pow( (1.0 - e1mu), 1.0/nexp2() );    
    }
    
  else if ( quad == LOWER_LEFT )
    {
      double x1 = pln();
      double ts = (pln() + ple() - lpiovl())/( 1.0 - lpiovl() );
      double x3 = (1.0 - ts)*(1.0 - lpiovl() );
      double x2 = 1.0 - x1 - x3;
        
      double ttas = ptas()*bu()/(1.0 - lpiovl());
      double zt = ttas*(1.0 - lpiovl() );
      double ttas2 = zt/x3;
      
      double c12 = ( x - lpiovl() )/( 1.0 - lpiovl() );
      double c11 = ( x - (x1+x2))/( 1.0 - (x1+x2));

      double glm = 1.0;
      double ltheta = atan( (bl() - ble())/(1.0 - pln() - ple()));
      if ( theta() != 0.0 && ltheta != 0.0 && ble() != bl() )
        glm = ((1.0 - pln() - ple())/fabs(bl() - ble()))*tan(theta()*DEG_2_RAD);
      glm = 1.0/glm;   
     
      double c2 = 0;
      double c3 = bl();
      if ( x >= (x1 + x2) ) 
        {
          c2 = -ttas2*(1.0 - (x1+x2))*c11;
          if ( fabs( c11 - 1.0 ) < 0.0001 )
            c11 = 1.0;
          c3 = ble() + ( bl() - ble() )*pow( (1.0-c11), 1.0/glm ); 
        }

      double teal = al()*pera();
      double c4 = c12*(teal-al()) + al();
      double e1 = y/c4;

      if ( e1 <= 0.0 )
        e1 = 0;
      
      //==== Dont Know What XFLRS is So Dont Check xxx-jrg ====//
      double e1ml = pow( e1, mexp3() );

      z = -(c2 + c3*pow( 1.0-e1ml, 1.0/nexp3() ));
      
    }
       
  else if ( quad == LOWER_RIGHT )
    {
      double x1 = pln();
      double ts = (pln() + ple() - lpiovl())/( 1.0 - lpiovl() );
      double x3 = (1.0 - ts)*(1.0 - lpiovl() );
      double x2 = 1.0 - x1 - x3;
        
      double ttas = ptas()*bu()/(1.0 - lpiovl());
      double zt = ttas*(1.0 - lpiovl() );
      double ttas2 = zt/x3;
      
      double c12 = ( x - lpiovl() )/( 1.0 - lpiovl() );
      double c11 = ( x - (x1+x2))/( 1.0 - (x1+x2));


      double glm = 1.0;
      double ltheta = atan( (bl() - ble())/(1.0 - pln() - ple()));
      if ( theta() != 0.0 && ltheta != 0.0 && ble() != bl() )
        glm = (1.0 - pln() - ple())/fabs(bl() - ble())*tan(theta()*DEG_2_RAD);
      glm = 1.0/glm;

      double c2 = 0;
      double c3 = bl();

      if ( x >= (x1 + x2) ) 
        {
          c2 = -ttas2*(1.0 - (x1+x2))*c11;

          if ( fabs( c11 - 1.0 ) < 0.0001 )
            c11 = 1.0;

          c3 = ble() + ( bl() - ble() )*pow( (1.0-c11), 1.0/glm ); 
        }
      
      double tear = ar()*pera();
      double c4 = c12*(tear-ar()) + ar();
      double e1 = y/c4;

      if ( e1 <= 0.0 )
        e1 = 0;
      
      //==== Dont Know What XFLRS is So Dont Check xxx-jrg ====//
      double e1ml = pow( e1, mexp4() );

      z = -(c2 + c3*pow( 1.0-e1ml, 1.0/nexp4() ));

    }
    
//	assert( z != infTest );
//	assert( z != nanTest );		jrg doesnt seem to work on win
//	assert( z > -1000000 && z < 1000000 );

  return z;
}


//==== Dump Xsec File =====//
void Havoc_geom::dump_xsec_file(int geom_no, FILE* dump_file)
{
  int i;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",body_surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",body_surf.get_num_pnts());

  for ( i = 0 ; i < body_surf.get_num_xsecs() ; i++ )
    {
      body_surf.write_xsec(i, model_mat, dump_file );
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",body_surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",body_surf.get_num_pnts());

  for ( i = 0 ; i < body_surf.get_num_xsecs() ; i++ )
    {
      body_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }

}

//==== Aero Ref Parameters ====//
double Havoc_geom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		refArea = get_area();
	}
	return refArea;
}
	
double Havoc_geom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		refSpan = get_span();
	}
	return refSpan;
}

double Havoc_geom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		refCbar = length();
	}
	return refCbar;
}

vec3d Havoc_geom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		double len = length()*scaleFactor(); 
		aeroCenter.set_x( len*0.25 + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void Havoc_geom::GetInteriorPnts( vector< vec3d > & pVec )
{
	vec3d p( 0.5*length(), 0.0, 0.0 );
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}



