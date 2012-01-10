//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//****************************************************************************
//    
//   Wing Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "havocScreen.h"
#include "havoc_geom.h"
#include "materialMgr.h"
#include "screenMgr.h"

//==== Constructor =====//
HavocScreen::HavocScreen(ScreenMgr* mgr) : CompScreen( mgr )
{

	HavocUI* ui = havocUI = new HavocUI();

	ui->UIWindow->position( 760, 30 );
	ui->UIWindow->callback( staticCloseCB, this );
	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
    #include "CommonParmButtonGui.h"

	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 10.0 );
	lengthInput  = new Input( this, ui->lengthInput );

	arSlider = new Slider_adj_range(this, ui->arSlider, ui->arButtonL, ui->arButtonR, 1.0 );
	arInput  = new Input( this, ui->arInput );
	alSlider = new Slider_adj_range(this, ui->alSlider, ui->alButtonL, ui->alButtonR, 1.0 );
	alInput  = new Input( this, ui->alInput );
	apmSlider = new Slider_adj_range(this, ui->apmSlider, ui->apmButtonL, ui->apmButtonR, 1.0 );
	apmInput  = new Input( this, ui->apmInput );
	apnSlider = new Slider_adj_range(this, ui->apnSlider, ui->apnButtonL, ui->apnButtonR, 1.0 );
	apnInput  = new Input( this, ui->apnInput );
	lpiovlSlider = new Slider_adj_range(this, ui->lpiovlSlider, ui->lpiovlButtonL, ui->lpiovlButtonR, 1.0 );
	lpiovlInput  = new Input( this, ui->lpiovlInput );
	peraSlider   = new Slider_adj_range(this, ui->peraSlider, ui->peraButtonL, ui->peraButtonR, 1.0 );
	peraInput    = new Input( this, ui->peraInput );

	mexp1Slider = new Slider_adj_range(this, ui->mexp1Slider, ui->mexp1ButtonL, ui->mexp1ButtonR, 1.0 );
	mexp1Input  = new Input( this, ui->mexp1Input );
	nexp1Slider = new Slider_adj_range(this, ui->nexp1Slider, ui->nexp1ButtonL, ui->nexp1ButtonR, 1.0 );
	nexp1Input  = new Input( this, ui->nexp1Input );
	mexp2Slider = new Slider_adj_range(this, ui->mexp2Slider, ui->mexp2ButtonL, ui->mexp2ButtonR, 1.0 );
	mexp2Input  = new Input( this, ui->mexp2Input );
	nexp2Slider = new Slider_adj_range(this, ui->nexp2Slider, ui->nexp2ButtonL, ui->nexp2ButtonR, 1.0 );
	nexp2Input  = new Input( this, ui->nexp2Input );
	mexp3Slider = new Slider_adj_range(this, ui->mexp3Slider, ui->mexp3ButtonL, ui->mexp3ButtonR, 1.0 );
	mexp3Input  = new Input( this, ui->mexp3Input );
	nexp3Slider = new Slider_adj_range(this, ui->nexp3Slider, ui->nexp3ButtonL, ui->nexp3ButtonR, 1.0 );
	nexp3Input  = new Input( this, ui->nexp3Input );
	mexp4Slider = new Slider_adj_range(this, ui->mexp4Slider, ui->mexp4ButtonL, ui->mexp4ButtonR, 1.0 );
	mexp4Input  = new Input( this, ui->mexp4Input );
	nexp4Slider = new Slider_adj_range(this, ui->nexp4Slider, ui->nexp4ButtonL, ui->nexp4ButtonR, 1.0 );
	nexp4Input  = new Input( this, ui->nexp4Input );

	plnSlider = new Slider_adj_range(this, ui->plnSlider, ui->plnButtonL, ui->plnButtonR, 1.0 );
	plnInput  = new Input( this, ui->plnInput );
	pleSlider = new Slider_adj_range(this, ui->pleSlider, ui->pleButtonL, ui->pleButtonR, 1.0 );
	pleInput  = new Input( this, ui->pleInput );
	buSlider = new Slider_adj_range(this, ui->buSlider, ui->buButtonL, ui->buButtonR, 1.0 );
	buInput  = new Input( this, ui->buInput );
	blSlider = new Slider_adj_range(this, ui->blSlider, ui->blButtonL, ui->blButtonR, 1.0 );
	blInput  = new Input( this, ui->blInput );

	umSlider = new Slider_adj_range(this, ui->umSlider, ui->umButtonL, ui->umButtonR, 1.0 );
	umInput  = new Input( this, ui->umInput );
	unSlider = new Slider_adj_range(this, ui->unSlider, ui->unButtonL, ui->unButtonR, 1.0 );
	unInput  = new Input( this, ui->unInput );

	lmSlider = new Slider_adj_range(this, ui->lmSlider, ui->lmButtonL, ui->lmButtonR, 1.0 );
	lmInput  = new Input( this, ui->lmInput );
	lnSlider = new Slider_adj_range(this, ui->lnSlider, ui->lnButtonL, ui->lnButtonR, 1.0 );
	lnInput  = new Input( this, ui->lnInput );

	gumSlider = new Slider_adj_range(this, ui->gumSlider, ui->gumButtonL, ui->gumButtonR, 1.0 );
	gumInput  = new Input( this, ui->gumInput );
	thetaSlider = new Slider_adj_range(this, ui->thetaSlider, ui->thetaButtonL, ui->thetaButtonR, 30.0 );
	thetaInput  = new Input( this, ui->thetaInput );
	ptasSlider = new Slider_adj_range(this, ui->ptasSlider, ui->ptasButtonL, ui->ptasButtonR, 1.0 );
	ptasInput  = new Input( this, ui->ptasInput );

	bueSlider = new Slider_adj_range(this, ui->bueSlider, ui->bueButtonL, ui->bueButtonR, 1.0 );
	bueInput  = new Input( this, ui->bueInput );
	bleSlider = new Slider_adj_range(this, ui->bleSlider, ui->bleButtonL, ui->bleButtonR, 1.0 );
	bleInput  = new Input( this, ui->bleInput );

	lengthButton = new ParmButton( this, ui->lengthlButton );
	alButton = new ParmButton( this, ui->alButton );
	arButton = new ParmButton( this, ui->arButton );
	apmButton = new ParmButton( this, ui->apmButton );
	apnButton = new ParmButton( this, ui->apnButton );
	lpiovlButton = new ParmButton( this, ui->lpiovlButton );
	peraButton = new ParmButton( this, ui->peraButton );

	mexp1Button = new ParmButton( this, ui->mexp1Button );
	mexp2Button = new ParmButton( this, ui->mexp2Button );
	mexp3Button = new ParmButton( this, ui->mexp3Button  );
	mexp4Button = new ParmButton( this, ui->mexp4Button  );
	nexp1Button = new ParmButton( this, ui->nexp1Button  );
	nexp2Button = new ParmButton( this, ui->nexp2Button  );
	nexp3Button = new ParmButton( this, ui->nexp3Button  );
	nexp4Button = new ParmButton( this, ui->nexp4Button  );

	plnButton = new ParmButton( this, ui->plnButton  );
	pleButton = new ParmButton( this, ui->pleButton  );
	buButton = new ParmButton( this, ui->buButton  );
	blButton = new ParmButton( this, ui->blButton  );
	umButton = new ParmButton( this, ui->umButton  );
	unButton = new ParmButton( this, ui->unButton  );
	lmButton = new ParmButton( this, ui->lmButton  );
	lnButton = new ParmButton( this, ui->lnButton  );
	gumButton = new ParmButton( this, ui->gumButton  );
	thetaButton = new ParmButton( this, ui->thetaButton  );
	ptasButton = new ParmButton( this, ui->ptasButton  );
	bueButton = new ParmButton( this, ui->bueButton  );
	bleButton = new ParmButton( this, ui->bleButton  );


	//==== Initail General Comp Screen UI ====//
	CompScreen::init();


}

//==== Destructor =====//
HavocScreen::~HavocScreen()
{
}

void HavocScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (Havoc_geom*)geomPtr;

	HavocUI* ui = havocUI;

	//==== Havoc Stuff ====//
	lengthSlider->set_parm_ptr( currGeom->get_length() );
	lengthInput->set_parm_ptr( currGeom->get_length() );
	alSlider->set_parm_ptr( currGeom->get_al() );
	alInput->set_parm_ptr( currGeom->get_al() );
	arSlider->set_parm_ptr( currGeom->get_ar() );
	arInput->set_parm_ptr( currGeom->get_ar() );
	apmSlider->set_parm_ptr( currGeom->get_apm() );
	apmInput->set_parm_ptr( currGeom->get_apm() );
	apnSlider->set_parm_ptr( currGeom->get_apn() );
	apnInput->set_parm_ptr( currGeom->get_apn() );

	lpiovlSlider->set_parm_ptr( currGeom->get_lpiovl() );
	lpiovlInput->set_parm_ptr( currGeom->get_lpiovl() );
	peraSlider->set_parm_ptr( currGeom->get_pera() );
	peraInput->set_parm_ptr( currGeom->get_pera() );

	mexp1Slider->set_parm_ptr( currGeom->get_mexp1() );
	mexp1Input->set_parm_ptr( currGeom->get_mexp1() );
	nexp1Slider->set_parm_ptr( currGeom->get_nexp1() );
	nexp1Input->set_parm_ptr( currGeom->get_nexp1() );
	mexp2Slider->set_parm_ptr( currGeom->get_mexp2() );
	mexp2Input->set_parm_ptr( currGeom->get_mexp2() );
	nexp2Slider->set_parm_ptr( currGeom->get_nexp2() );
	nexp2Input->set_parm_ptr( currGeom->get_nexp2() );

	mexp3Slider->set_parm_ptr( currGeom->get_mexp3() );
	mexp3Input->set_parm_ptr( currGeom->get_mexp3() );
	nexp3Slider->set_parm_ptr( currGeom->get_nexp3() );
	nexp3Input->set_parm_ptr( currGeom->get_nexp3() );
	mexp4Slider->set_parm_ptr( currGeom->get_mexp4() );
	mexp4Input->set_parm_ptr( currGeom->get_mexp4() );
	nexp4Slider->set_parm_ptr( currGeom->get_nexp4() );
	nexp4Input->set_parm_ptr( currGeom->get_nexp4() );

	plnSlider->set_parm_ptr( currGeom->get_pln() );
	plnInput->set_parm_ptr( currGeom->get_pln() );
	pleSlider->set_parm_ptr( currGeom->get_ple() );
	pleInput->set_parm_ptr( currGeom->get_ple() );
	buSlider->set_parm_ptr( currGeom->get_bu() );
	buInput->set_parm_ptr( currGeom->get_bu() );
	blSlider->set_parm_ptr( currGeom->get_bl() );
	blInput->set_parm_ptr( currGeom->get_bl() );

	umSlider->set_parm_ptr( currGeom->get_aum() );
	umInput->set_parm_ptr( currGeom->get_aum() );
	unSlider->set_parm_ptr( currGeom->get_aun() );
	unInput->set_parm_ptr( currGeom->get_aun() );

	lmSlider->set_parm_ptr( currGeom->get_alm() );
	lmInput->set_parm_ptr( currGeom->get_alm() );
	lnSlider->set_parm_ptr( currGeom->get_aln() );
	lnInput->set_parm_ptr( currGeom->get_aln() );

	gumSlider->set_parm_ptr( currGeom->get_gum() );
	gumInput->set_parm_ptr( currGeom->get_gum() );
	thetaSlider->set_parm_ptr( currGeom->get_theta() );
	thetaInput->set_parm_ptr( currGeom->get_theta() );
	ptasSlider->set_parm_ptr( currGeom->get_ptas() );
	ptasInput->set_parm_ptr( currGeom->get_ptas() );

	bueSlider->set_parm_ptr( currGeom->get_bue() );
	bueInput->set_parm_ptr( currGeom->get_bue() );
	bleSlider->set_parm_ptr( currGeom->get_ble() );
	bleInput->set_parm_ptr( currGeom->get_ble() );

	lengthButton->set_parm_ptr( currGeom->get_length() );
	alButton->set_parm_ptr( currGeom->get_al() );
	arButton->set_parm_ptr( currGeom->get_ar() );
	apmButton->set_parm_ptr( currGeom->get_apm() );
	apnButton->set_parm_ptr( currGeom->get_apn() );
	lpiovlButton->set_parm_ptr( currGeom->get_lpiovl() );
	peraButton->set_parm_ptr( currGeom->get_pera() );

	mexp1Button->set_parm_ptr( currGeom->get_mexp1() );
	nexp1Button->set_parm_ptr( currGeom->get_nexp1() );
	mexp2Button->set_parm_ptr( currGeom->get_mexp2() );
	nexp2Button->set_parm_ptr( currGeom->get_nexp2() );

	mexp3Button->set_parm_ptr( currGeom->get_mexp3() );
	nexp3Button->set_parm_ptr( currGeom->get_nexp3() );
	mexp4Button->set_parm_ptr( currGeom->get_mexp4() );
	nexp4Button->set_parm_ptr( currGeom->get_nexp4() );

	plnButton->set_parm_ptr( currGeom->get_pln() );
	pleButton->set_parm_ptr( currGeom->get_ple() );
	buButton->set_parm_ptr( currGeom->get_bu() );
	blButton->set_parm_ptr( currGeom->get_bl() );

	umButton->set_parm_ptr( currGeom->get_aum() );
	unButton->set_parm_ptr( currGeom->get_aun() );

	lmButton->set_parm_ptr( currGeom->get_alm() );
	lnButton->set_parm_ptr( currGeom->get_aln() );

	gumButton->set_parm_ptr( currGeom->get_gum() );
	thetaButton->set_parm_ptr( currGeom->get_theta() );
	ptasButton->set_parm_ptr( currGeom->get_ptas() );

	bueButton->set_parm_ptr( currGeom->get_bue() );
	bleButton->set_parm_ptr( currGeom->get_ble() );


	ui->UIWindow->show();

}

void HavocScreen::hide()
{
	havocUI->UIWindow->hide();
}

void HavocScreen::position( int x, int y )
{
	havocUI->UIWindow->position( x, y );
}

void HavocScreen::setTitle( const char* name )
{
	title = "HAVOC : ";
	title.concatenate( name );

	havocUI->TitleBox->label( title );
}

void HavocScreen::parm_changed( Parm* parm )
{
}


//==== Close Callbacks =====//
void HavocScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	havocUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void HavocScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

}

