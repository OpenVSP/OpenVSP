//****************************************************************************
//    
//   Hrm Geom Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "hrmScreen.h"
#include "propScreen.h"
#include "vorGeom.h"
#include "screenMgr.h"
#include "scriptMgr.h"


//==== Constructor =====//
HrmScreen::HrmScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	HrmUI* ui = hrmUI = new HrmUI();

	hrmUI->UIWindow->position( 760, 30 );
	hrmUI->UIWindow->callback( staticCloseCB, this );
	
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

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
HrmScreen::~HrmScreen()
{
}

void HrmScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (VorGeom*)geomPtr;
	HrmUI* ui = hrmUI;

	setTitle( currGeom->getName().get_char_star() );

	ui->UIWindow->show();

}

void HrmScreen::hide()
{
	hrmUI->UIWindow->hide();
}

void HrmScreen::position( int x, int y )
{
	hrmUI->UIWindow->position( x, y );
}

void HrmScreen::setTitle( const char* name )
{
	title = "HRM : ";
	title.concatenate( name );

	hrmUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void HrmScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	hrmUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void HrmScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

}

