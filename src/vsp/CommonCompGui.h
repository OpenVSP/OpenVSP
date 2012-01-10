//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

{
	
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

	densityInput = new Input( this, ui->densityInput );
	shellMassAreaInput = new Input( this, ui->shellMassAreaInput );
	shellEnableButton = ui->shellEnableButton;
	priorityCounter = ui->priorityCounter;


}

