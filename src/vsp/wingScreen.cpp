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

#include "wingScreen.h"
#include "materialMgr.h"
#include "screenMgr.h"
#include "af.h"

//==== Constructor =====//
WingScreen::WingScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	WingUI* ui = wingUI = new WingUI();

	wingUI->UIWindow->position( 760, 30 );
	wingUI->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	xLocSlider = new Slider_adj_range(this, ui->xLocSlider, ui->xLocButtonL, ui->xLocButtonR, 10.0 );
	yLocSlider = new Slider_adj_range(this, ui->yLocSlider, ui->yLocButtonL, ui->yLocButtonR, 10.0 );
	zLocSlider = new Slider_adj_range(this, ui->zLocSlider, ui->zLocButtonL, ui->zLocButtonR, 10.0 );
	xLocInput = new Input( this, ui->xLocInput );
	yLocInput = new Input( this, ui->yLocInput );
	zLocInput = new Input( this, ui->zLocInput );

	xRotSlider = new Slider_adj_range(this, ui->xRotSlider, ui->xRotButtonL, ui->xRotButtonR, 90.0 );
	yRotSlider = new Slider_adj_range(this, ui->yRotSlider, ui->yRotButtonL, ui->yRotButtonR, 90.0 );
	zRotSlider = new Slider_adj_range(this, ui->zRotSlider, ui->zRotButtonL, ui->zRotButtonR, 90.0 );
	xRotInput = new Input( this, ui->xRotInput );
	yRotInput = new Input( this, ui->yRotInput );
	zRotInput = new Input( this, ui->zRotInput );

	rotOriginSlider = new Slider_adj_range(this, ui->rotOriginSlider, ui->rotOriginButtonL, ui->rotOriginButtonR, 1.0 );
	rotOriginInput = new Input( this, ui->rotOriginInput );

	uSlider = new Slider(this, ui->uSlider );
	vSlider = new Slider(this, ui->vSlider );
	uInput = new Input( this, ui->uInput );
	vInput = new Input( this, ui->vInput );

	//==== Init CompScreen UI - IN COMP SCREEN ====//
	nameInput       = ui->nameInput;
	colorButton     = ui->colorButton;
	primColorButton = ui->primColorButton;
	materialChoice  = ui->materialChoice;
	attachUVButton  = ui->attachUVButton;
	attachFixedButton = ui->attachFixedButton;
	attachMatrixButton = ui->attachMatrixButton;

	redSlider   = ui->redSlider;
	greenSlider = ui->greenSlider;
	blueSlider  = ui->blueSlider;
	redButton     = ui->redButton;
	greenButton   = ui->greenButton;
	blueButton    = ui->blueButton;
	yellowButton  = ui->yellowButton;
	magentaButton = ui->magentaButton;
	cyanButton    = ui->cyanButton;
	blackButton   = ui->blackButton;
	darkBlueButton= ui->darkBlueButton;
	greyButton    = ui->greyButton;

	symmetryChoice = ui->symmetryChoice;
	acceptScaleButton = ui->acceptScaleButton;
	resetScaleButton  = ui->resetScaleButton;

	noneSymMenu = ui->noneSymMenu;
	xySymMenu   = ui->xySymMenu;
	yzSymMenu   = ui->yzSymMenu;
	xzSymMenu   = ui->xzSymMenu;

	absButton = ui->absButton;
	relButton = ui->relButton;

	scaleSlider = new LogSlider(this, ui->scaleSlider );
	scaleInput  = new Input( this, ui->scaleInput );

	numXsecsSlider = new Slider( this, ui->numXsecsSlider );
	numXsecsInput  = new Input( this, ui->numXsecsInput );
	numXsecsInput->set_format( "%1.0f" );

	numPntsSlider = new Slider( this, ui->numPointsSlider );
	numPntsInput  = new Input( this, ui->numPointsInput );
	numPntsInput->set_format( "%1.0f" );

	outputEnableButton = ui->outputEnableButton;
	outputNameChoice = ui->outputNameChoice;
	//==============================================// 
	//==============================================// 

	//==== Wing Stuff ====//
	ARSlider = new Slider_adj_range(this, ui->ARSlider, ui->ARButtonL, ui->ARButtonR, 5.0 );
	ARInput = new Input( this, ui->ARInput );
	TRSlider = new Slider_adj_range(this, ui->TRSlider, ui->TRButtonL, ui->TRButtonR, 1.0 );
	TRInput = new Input( this, ui->TRInput );
	areaSlider = new Slider_adj_range(this, ui->areaSlider, ui->areaButtonL, ui->areaButtonR, 500.0 );
	areaInput = new Input( this, ui->areaInput );
	spanSlider = new Slider_adj_range(this, ui->spanSlider, ui->spanButtonL, ui->spanButtonR, 50.0 );
	spanInput = new Input( this, ui->spanInput );
	TCSlider = new Slider_adj_range(this, ui->TCSlider, ui->TCButtonL, ui->TCButtonR, 50.0 );
	TCInput = new Input( this, ui->TCInput );
	RCSlider = new Slider_adj_range(this, ui->RCSlider, ui->RCButtonL, ui->RCButtonR, 50.0 );
	RCInput = new Input( this, ui->RCInput );

	ui->driverChoice->callback( staticScreenCB, this );

	totalWingAreaInput = new Input( this, ui->totalWingInput );

	sweepSlider = new Slider_adj_range(this, ui->sweepSlider, ui->sweepButtonL, ui->sweepButtonR, 30.0 );
	sweepInput = new Input( this, ui->sweepInput );

	sweepLocSlider = new Slider_adj_range(this, ui->sweepLocSlider, ui->sweepLocButtonL, ui->sweepLocButtonR, 1.0 );
	sweepLocInput = new Input( this, ui->sweepLocInput );

	twistLocSlider = new Slider_adj_range(this, ui->twistLocSlider, ui->twistLocButtonL, ui->twistLocButtonR, 1.0 );
	twistLocInput = new Input( this, ui->twistLocInput );

	inTwistSlider = new Slider_adj_range(this, ui->twistInSlider, ui->twistInButtonL, ui->twistInButtonR, 10.0 );
	inTwistInput = new Input( this, ui->twistInInput );
	inDihedSlider = new Slider_adj_range(this, ui->dihedInSlider, ui->dihedInButtonL, ui->dihedInButtonR, 10.0 );
	inDihedInput = new Input( this, ui->dihedInInput );

	midTwistSlider = new Slider_adj_range(this, ui->twistMidSlider, ui->twistMidButtonL, ui->twistMidButtonR, 10.0 );
	midTwistInput = new Input( this, ui->twistMidInput );
	midDihedSlider = new Slider_adj_range(this, ui->dihedMidSlider, ui->dihedMidButtonL, ui->dihedMidButtonR, 10.0 );
	midDihedInput = new Input( this, ui->dihedMidInput );

	outTwistSlider = new Slider_adj_range(this, ui->twistOutSlider, ui->twistOutButtonL, ui->twistOutButtonR, 10.0 );
	outTwistInput = new Input( this, ui->twistOutInput );
	outDihedSlider = new Slider_adj_range(this, ui->dihedOutSlider, ui->dihedOutButtonL, ui->dihedOutButtonR, 10.0 );
	outDihedInput = new Input( this, ui->dihedOutInput );

	strakeSweepSlider = new Slider_adj_range(this, ui->sweepStrakeSlider, ui->sweepStrakeButtonL, ui->sweepStrakeButtonR, 30.0 );
	strakeSweepInput = new Input( this, ui->sweepStrakeInput );
	strakeSpanSlider = new Slider_adj_range(this, ui->spanStrakeSlider, ui->spanStrakeButtonL, ui->spanStrakeButtonR, 45.0 );
	strakeSpanInput = new Input( this, ui->spanStrakeInput );

	aftSweepSlider = new Slider_adj_range(this, ui->sweepAftSlider, ui->sweepAftButtonL, ui->sweepAftButtonR, 30.0 );
	aftSweepInput = new Input( this, ui->sweepAftInput );
	aftSpanSlider = new Slider_adj_range(this, ui->spanAftSlider, ui->spanAftButtonL, ui->spanAftButtonR, 45.0 );
	aftSpanInput = new Input( this, ui->spanAftInput );

	ui->onButton->callback( staticScreenCB, this );
	ui->offButton->callback( staticScreenCB, this );


	deflScaleSlider = new Slider_adj_range(this, ui->deflScaleSlider, ui->deflScaleButtonL, ui->deflScaleButtonR, 45.0 );
	deflScaleInput = new Input( this, ui->deflScaleInput );
	twistScaleSlider = new Slider_adj_range(this, ui->twistScaleSlider, ui->twistScaleButtonL, ui->twistScaleButtonR, 45.0 );
	twistScaleInput = new Input( this, ui->twistScaleInput );
	
	ui->deflOnButton->callback( staticScreenCB, this );
	ui->deflOffButton->callback( staticScreenCB, this );
	ui->readDeflButton->callback( staticScreenCB, this );

	//==== Airfoil Stuff ====//
	ui->airfoilNameInput->readonly(1);
	ui->sectionChoice->callback( staticScreenCB, this );
	ui->airfoilTypeChoice->callback( staticScreenCB, this );
	ui->readFileButton->callback( staticScreenCB, this );
	ui->sixSeriesChoice->callback( staticScreenCB, this );
	ui->airfoilInvertButton->callback( staticScreenCB, this );

	afCamberSlider = new Slider_adj_range(this, ui->camberSlider, ui->camberButtonL, ui->camberButtonR, 45.0 );
	afCamberInput = new Input( this, ui->camberInput );
	afCamberLocSlider = new Slider_adj_range(this, ui->camberLocSlider, ui->camberLocButtonL, ui->camberLocButtonR, 45.0 );
	afCamberLocInput = new Input( this, ui->camberLocInput );
	afThickSlider = new Slider_adj_range(this, ui->thickSlider, ui->thickButtonL, ui->thickButtonR, 45.0 );
	afThickInput = new Input( this, ui->thickInput );
	afThickLocSlider = new Slider_adj_range(this, ui->thickLocSlider, ui->thickLocButtonL, ui->thickLocButtonR, 45.0 );
	afThickLocInput = new Input( this, ui->thickLocInput );
	afIdealClSlider = new Slider_adj_range(this, ui->idealClSlider, ui->idealClButtonL, ui->idealClButtonR, 45.0 );
	afIdealClInput = new Input( this, ui->idealClInput );
	afASlider = new Slider_adj_range(this, ui->aSlider, ui->aButtonL, ui->aButtonR, 45.0 );
	afAInput = new Input( this, ui->aInput );

	leRadiusInput = new Input( this, ui->leRadInput );

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->airfoilGlWinGroup;
	ui->airfoilGlWinGroup->begin();
	glWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->airfoilGlWinGroup->end();

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
WingScreen::~WingScreen()
{
}

void WingScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (WingGeom*)geomPtr;
	setTitle( currGeom->getName().get_char_star() );


	ARSlider->set_parm_ptr( currGeom->get_aspect() );
	ARInput->set_parm_ptr( currGeom->get_aspect() );
	TRSlider->set_parm_ptr( currGeom->get_taper() );
	TRInput->set_parm_ptr( currGeom->get_taper() );
	areaSlider->set_parm_ptr( currGeom->get_mw_area() );
	areaInput->set_parm_ptr( currGeom->get_mw_area() );
	spanSlider->set_parm_ptr( currGeom->get_mw_span() );
	spanInput->set_parm_ptr( currGeom->get_mw_span() );
	RCSlider->set_parm_ptr( currGeom->get_root_chord() );
	RCInput->set_parm_ptr( currGeom->get_root_chord() );
	TCSlider->set_parm_ptr( currGeom->get_tip_chord() );
	TCInput->set_parm_ptr( currGeom->get_tip_chord() );

	wingUI->driverChoice->value( currGeom->get_driver() );

	sweepSlider->set_parm_ptr( currGeom->get_sweep() );
	sweepInput->set_parm_ptr( currGeom->get_sweep() );

	sweepLocSlider->set_parm_ptr( currGeom->get_sweep_loc() );
	sweepLocInput->set_parm_ptr( currGeom->get_sweep_loc() );

	totalWingAreaInput->set_parm_ptr( currGeom->get_total_area() );

	twistLocSlider->set_parm_ptr( currGeom->get_twist_loc() );
	twistLocInput->set_parm_ptr( currGeom->get_twist_loc() );

	inTwistSlider->set_parm_ptr( currGeom->get_in_twist() );
	inTwistInput->set_parm_ptr( currGeom->get_in_twist() );
	inDihedSlider->set_parm_ptr( currGeom->get_in_dihed() );
	inDihedInput->set_parm_ptr( currGeom->get_in_dihed() );

	midTwistSlider->set_parm_ptr( currGeom->get_mid_twist() );
	midTwistInput->set_parm_ptr( currGeom->get_mid_twist() );
	midDihedSlider->set_parm_ptr( currGeom->get_mid_dihed() );
	midDihedInput->set_parm_ptr( currGeom->get_mid_dihed() );

	outTwistSlider->set_parm_ptr( currGeom->get_out_twist() );
	outTwistInput->set_parm_ptr( currGeom->get_out_twist() );
	outDihedSlider->set_parm_ptr( currGeom->get_out_dihed() );
	outDihedInput->set_parm_ptr( currGeom->get_out_dihed() );

	strakeSweepSlider->set_parm_ptr( currGeom->get_strake_sweep() );
	strakeSweepInput->set_parm_ptr( currGeom->get_strake_sweep() );
	strakeSpanSlider->set_parm_ptr( currGeom->get_strake_span_per() );
	strakeSpanInput->set_parm_ptr( currGeom->get_strake_span_per() );
	aftSweepSlider->set_parm_ptr( currGeom->get_aft_ext_sweep() );
	aftSweepInput->set_parm_ptr( currGeom->get_aft_ext_sweep() );
	aftSpanSlider->set_parm_ptr( currGeom->get_aft_ext_span_per() );
	aftSpanInput->set_parm_ptr( currGeom->get_aft_ext_span_per() );

	if ( currGeom->get_strake_aft_flag() )
	{
		wingUI->onButton->value( 1 );
		wingUI->offButton->value( 0 );
	}
	else
	{
		wingUI->onButton->value( 0 );
		wingUI->offButton->value( 1 );
	}

	deflScaleSlider->set_parm_ptr( currGeom->get_defl_scale() );
	deflScaleInput->set_parm_ptr( currGeom->get_defl_scale() );
	twistScaleSlider->set_parm_ptr( currGeom->get_twist_scale() );
	twistScaleInput->set_parm_ptr( currGeom->get_twist_scale() );

	if ( currGeom->get_deflect_flag() )
	{
		wingUI->deflOnButton->value( 1 );
		wingUI->deflOffButton->value( 0 );
	}
	else
	{
		wingUI->deflOnButton->value( 0 );
		wingUI->deflOffButton->value( 1 );
	}
	wingUI->readDeflInput->value( currGeom->get_deflect_file_name() );

	//==== Airfoil Stuff ====//
	Af* afp = currGeom->get_af_ptr();
	wingUI->sectionChoice->value( currGeom->get_curr_af_id() );
	wingUI->airfoilTypeChoice->value( afp->get_type()-1 );
	wingUI->airfoilInvertButton->value( afp->get_inverted_flag() );

	afCamberSlider->set_parm_ptr( afp->get_camber() );
	afCamberInput->set_parm_ptr(  afp->get_camber() );
	afCamberLocSlider->set_parm_ptr( afp->get_camber_loc() );
	afCamberLocInput->set_parm_ptr(  afp->get_camber_loc() );
	afThickSlider->set_parm_ptr( afp->get_thickness() );
	afThickInput->set_parm_ptr(  afp->get_thickness() );
	afThickLocSlider->set_parm_ptr( afp->get_thickness_loc() );
	afThickLocInput->set_parm_ptr(  afp->get_thickness_loc() );
	afIdealClSlider->set_parm_ptr( afp->get_ideal_cl() );
	afIdealClInput->set_parm_ptr(  afp->get_ideal_cl() );
	afASlider->set_parm_ptr( afp->get_a() );
	afAInput->set_parm_ptr(  afp->get_a() );

	wingUI->airfoilNameInput->value( afp->get_name() );

	if ( afp->get_type() == NACA_6_SERIES )
		wingUI->sixSeriesChoice->activate();
	else
		wingUI->sixSeriesChoice->deactivate();

	leRadiusInput->set_parm_ptr( afp->get_leading_edge_radius() );


	glWin->setDrawBase( afp );
	glWin->redraw();


	wingUI->UIWindow->show();

}

void WingScreen::hide()
{
	wingUI->UIWindow->hide();
}

void WingScreen::position( int x, int y )
{
	wingUI->UIWindow->position( x, y );
}

void WingScreen::setTitle( const char* name )
{
	title = "WING : ";
	title.concatenate( name );

	wingUI->TitleBox->label( title );
}

void WingScreen::parm_changed( Parm* parm )
{
	if ( parm )
	{
		if ( parm->get_update_grp() == AF_UPDATE_GROUP )
		{
			updateName();
			glWin->redraw();
		}
	}
}

void WingScreen::updateName()
{
	Af* afp = currGeom->get_af_ptr();
	wingUI->airfoilNameInput->value( afp->get_name() );

}

//==== Close Callbacks =====//
void WingScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	wingUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void WingScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

	if ( w == wingUI->driverChoice )
	{
		currGeom->set_driver( wingUI->driverChoice->value() );
	}
	else if ( w == wingUI->onButton )
	{
		currGeom->set_strake_aft_flag( 1 );
		wingUI->offButton->value( 0 );
	}
	else if ( w == wingUI->offButton )
	{
		currGeom->set_strake_aft_flag( 0 );
		wingUI->onButton->value( 0 );
	}
	else if ( w == wingUI->deflOnButton )
	{
		currGeom->set_deflect_flag(1);
		wingUI->deflOffButton->value(0);
	}
	else if ( w == wingUI->deflOffButton )
	{
		currGeom->set_deflect_flag(0);
		wingUI->deflOnButton->value(0);
	}
	else if ( w == wingUI->readDeflButton )
	{
//		char *newfile = fl_file_chooser("Read Deflection File?", "*.def", "");
		char *newfile = screenMgrPtr->FileChooser("Read Deflection File?", "*.def");

		if ( newfile != NULL )
		{
			currGeom->read_deflect_file( Stringc(newfile) );

		}
		wingUI->readDeflInput->value( currGeom->get_deflect_file_name() );
	}
	else if ( w == wingUI->sectionChoice )
	{
		currGeom->set_curr_af_id( wingUI->sectionChoice->value() );
		show( currGeom );
	}
	else if ( w == wingUI->airfoilTypeChoice )
	{
		int aftype = wingUI->airfoilTypeChoice->value()+1;

		if ( aftype == AIRFOIL_FILE )
		{
//			char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
			char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
			if ( newfile != NULL )
			{
				currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
			}
			//==== Force Update and Redraw ===//
			currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
			show( currGeom );
		}
		else 
		{
			currGeom->get_af_ptr()->set_type( aftype );

			//==== Force Update and Redraw ===//
			currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
			show( currGeom );
		}
	}
	else if ( w ==  wingUI->readFileButton )
	{
//		char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
		char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
		if ( newfile != NULL )
		{
			currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
		}
		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  wingUI->sixSeriesChoice  )
	{
		int choice = wingUI->sixSeriesChoice->value();

		if ( choice <= 4 )
			currGeom->get_af_ptr()->set_sixser( choice + 63 );
		else
			currGeom->get_af_ptr()->set_sixser( -(choice + 58) );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

	}	
	else if ( w ==  wingUI->airfoilInvertButton   )
	{
		int flag = wingUI->airfoilInvertButton->value();

		currGeom->get_af_ptr()->set_inverted_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

	}

}



