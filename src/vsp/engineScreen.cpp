//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//****************************************************************************
//    
//   Engine Geom Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "engineScreen.h"
#include "engineGeom.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//==== Constructor =====//
EngineScreen::EngineScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	EngineUI* ui = engineUI = new EngineUI();

	engineUI->UIWindow->position( 760, 30 );
	engineUI->UIWindow->callback( staticCloseCB, this );
	
	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	//===== Engine Stuff ====//
	radTipSlider = new Slider_adj_range(this, ui->rtipSlider, ui->rtipButtonL, ui->rtipButtonR, 10.0 );
	radTipInput  = new Input( this, ui->rtipInput );
	maxTipSlider = new Slider_adj_range(this, ui->maxSlider, ui->maxButtonL, ui->maxButtonR, 10.0 );
	maxTipInput  = new Input( this, ui->maxInput );
	hubTipSlider = new Slider_adj_range(this, ui->hubSlider, ui->hubButtonL, ui->hubButtonR, 10.0 );
	hubTipInput  = new Input( this, ui->hubInput );
	lengthSlider = new Slider_adj_range(this, ui->lenrtipSlider, ui->lenrtipButtonL, ui->lenrtipButtonR, 10.0 );
	lengthInput  = new Input( this, ui->lenrtipInput );

	exitAreaSlider = new Slider_adj_range(this, ui->nozExitSlider, ui->nozExitButtonL, ui->nozExitButtonR, 10.0 );
	exitAreaInput  = new Input( this, ui->nozExitInput );
	nozLengthSlider = new Slider_adj_range(this, ui->nozlenSlider, ui->nozlenButtonL, ui->nozlenButtonR, 10.0 );
	nozLengthInput  = new Input( this, ui->nozlenInput );

	ductXOffSlider = new Slider_adj_range(this, ui->ductXoffSlider, ui->ductXoffButtonL, ui->ductXoffButtonR, 10.0 );
	ductXOffInput  = new Input( this, ui->ductXoffInput );
	ductYOffSlider = new Slider_adj_range(this, ui->ductYoffSlider, ui->ductYoffButtonL, ui->ductYoffButtonR, 10.0 );
	ductYOffInput  = new Input( this, ui->ductYoffInput );
	ductShapeSlider = new Slider_adj_range(this, ui->shapeSlider, ui->shapeButtonL, ui->shapeButtonR, 10.0 );
	ductShapeInput  = new Input( this, ui->shapeInput );

	ui->ductOffButton->callback( staticScreenCB, this );
	ui->ductOnButton->callback( staticScreenCB, this );

	cowlLenSlider = new Slider_adj_range(this, ui->cowlLenSlider, ui->cowlLenButtonL, ui->cowlLenButtonR, 10.0 );
	cowlLenInput  = new Input( this, ui->cowlLenInput);
	engThrtRatioSlider = new Slider_adj_range(this, ui->engThrtSlider, ui->engThrtButtonL, ui->engThrtButtonR, 10.0 );
	engThrtRatioInput  = new Input( this, ui->engThrtInput);
	hlThrtRatioSlider = new Slider_adj_range(this, ui->hlThrtSlider, ui->hlThrtButtonL, ui->hlThrtButtonR, 10.0 );
	hlThrtRatioInput  = new Input( this, ui->hlThrtInput);
	lipFineSlider = new Slider_adj_range(this, ui->lipFRSlider, ui->lipFRButtonL, ui->lipFRButtonR, 10.0 );
	lipFineInput  = new Input( this, ui->lipFRInput);
	hwRatioSlider = new Slider_adj_range(this, ui->hwSlider, ui->hwButtonL, ui->hwButtonR, 10.0 );
	hwRatioInput  = new Input( this, ui->hwInput);
	upShapeSlider = new Slider_adj_range(this, ui->upSurfSlider, ui->upSurfButtonL, ui->upSurfButtonR, 10.0 );
	upShapeInput  = new Input( this, ui->upSurfInput);
	lowShapeSlider = new Slider_adj_range(this, ui->lowSurfSlider, ui->lowSurfButtonL, ui->lowSurfButtonR, 10.0 );
	lowShapeInput  = new Input( this, ui->lowSurfInput);
	inlXRotSlider = new Slider_adj_range(this, ui->inletXRotSlider, ui->inletXRotButtonL, ui->inletXRotButtonR, 10.0 );
	inlXRotInput  = new Input( this, ui->inletXRotInput);
	scarfSlider = new Slider_adj_range(this, ui->scarfSlider, ui->scarfButtonL, ui->scarfButtonR, 10.0 );
	scarfInput  = new Input( this, ui->scarfInput);

	radTipButton = new ParmButton( this, ui->rtipButton );
	maxTipButton = new ParmButton( this, ui->maxButton );
	hubTipButton = new ParmButton( this, ui->hubButton  );
	lengthButton = new ParmButton( this, ui->lenrtipButton  );
	exitAreaButton = new ParmButton( this, ui->nozExitButton  );
	nozLengthButton = new ParmButton( this, ui->nozlenButton  );
	ductXOffButton = new ParmButton( this, ui->ductXoffButton  );
	ductYOffButton = new ParmButton( this, ui->ductYoffButton  );
	ductShapeButton = new ParmButton( this, ui->shapeButton  );
	cowlLenButton = new ParmButton( this, ui->cowlLenButton  );
	engThrtRatioButton = new ParmButton( this, ui->engThrtButton  );
	hlThrtRatioButton = new ParmButton( this, ui->hlThrtButton  );
	lipFineButton = new ParmButton( this, ui->lipFRButton  );
	hwRatioButton = new ParmButton( this, ui->hwButton  );
	upShapeButton = new ParmButton( this, ui->upSurfButton  );
	lowShapeButton = new ParmButton( this, ui->lowSurfButton  );
	inlXRotButton = new ParmButton( this, ui->inletXRotButton  );
	scarfButton = new ParmButton( this, ui->scarfButton  );


	//==== Initail General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
EngineScreen::~EngineScreen()
{
}

void EngineScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (EngineGeom*)geomPtr;
	EngineUI* ui = engineUI;

	setTitle( currGeom->getName().get_char_star() );

	lengthSlider->set_parm_ptr( currGeom->get_length() );
	lengthInput->set_parm_ptr( currGeom->get_length() );
	radTipSlider->set_parm_ptr( currGeom->get_rad_tip() );
	radTipInput->set_parm_ptr( currGeom->get_rad_tip() );
	maxTipSlider->set_parm_ptr( currGeom->get_max_tip() );
	maxTipInput->set_parm_ptr( currGeom->get_max_tip() );
	hubTipSlider->set_parm_ptr( currGeom->get_hub_tip() );
	hubTipInput->set_parm_ptr( currGeom->get_hub_tip() );

	exitAreaSlider->set_parm_ptr( currGeom->get_exit_area_ratio() );
	exitAreaInput->set_parm_ptr( currGeom->get_exit_area_ratio() );
	nozLengthSlider->set_parm_ptr( currGeom->get_noz_length() );
	nozLengthInput->set_parm_ptr( currGeom->get_noz_length() );

	ductXOffSlider->set_parm_ptr( currGeom->get_inl_duct_x_off() );
	ductXOffInput->set_parm_ptr( currGeom->get_inl_duct_x_off() );
	ductYOffSlider->set_parm_ptr( currGeom->get_inl_duct_y_off() );
	ductYOffInput->set_parm_ptr( currGeom->get_inl_duct_y_off() );
	ductShapeSlider->set_parm_ptr( currGeom->get_inl_duct_shape() );
	ductShapeInput->set_parm_ptr( currGeom->get_inl_duct_shape() );

	cowlLenSlider->set_parm_ptr( currGeom->get_cowl_length() );
	cowlLenInput->set_parm_ptr( currGeom->get_cowl_length() );
	engThrtRatioSlider->set_parm_ptr( currGeom->get_area_ratio_eng_thrt() );
	engThrtRatioInput->set_parm_ptr( currGeom->get_area_ratio_eng_thrt() );
	hlThrtRatioSlider->set_parm_ptr( currGeom->get_area_ratio_hl_thrt() );
	hlThrtRatioInput->set_parm_ptr( currGeom->get_area_ratio_hl_thrt() );
	lipFineSlider->set_parm_ptr( currGeom->get_lip_fine() );
	lipFineInput->set_parm_ptr( currGeom->get_lip_fine() );
	hwRatioSlider->set_parm_ptr( currGeom->get_ht_wid_ratio() );
	hwRatioInput->set_parm_ptr( currGeom->get_ht_wid_ratio() );
	upShapeSlider->set_parm_ptr( currGeom->get_upper_shape() );
	upShapeInput->set_parm_ptr( currGeom->get_upper_shape() );
	lowShapeSlider->set_parm_ptr( currGeom->get_lower_shape() );
	lowShapeInput->set_parm_ptr( currGeom->get_lower_shape() );
	inlXRotSlider->set_parm_ptr( currGeom->get_inl_x_rot() );
	inlXRotInput->set_parm_ptr( currGeom->get_inl_x_rot() );
	scarfSlider->set_parm_ptr( currGeom->get_inl_scarf() );
	scarfInput->set_parm_ptr( currGeom->get_inl_scarf() );

	lengthButton->set_parm_ptr( currGeom->get_length() );
	radTipButton->set_parm_ptr( currGeom->get_rad_tip() );
	maxTipButton->set_parm_ptr( currGeom->get_max_tip() );
	hubTipButton->set_parm_ptr( currGeom->get_hub_tip() );

	exitAreaButton->set_parm_ptr( currGeom->get_exit_area_ratio() );
	nozLengthButton->set_parm_ptr( currGeom->get_noz_length() );

	ductXOffButton->set_parm_ptr( currGeom->get_inl_duct_x_off() );
	ductYOffButton->set_parm_ptr( currGeom->get_inl_duct_y_off() );
	ductShapeButton->set_parm_ptr( currGeom->get_inl_duct_shape() );

	cowlLenButton->set_parm_ptr( currGeom->get_cowl_length() );
	engThrtRatioButton->set_parm_ptr( currGeom->get_area_ratio_eng_thrt() );
	hlThrtRatioButton->set_parm_ptr( currGeom->get_area_ratio_hl_thrt() );
	lipFineButton->set_parm_ptr( currGeom->get_lip_fine() );
	hwRatioButton->set_parm_ptr( currGeom->get_ht_wid_ratio() );
	upShapeButton->set_parm_ptr( currGeom->get_upper_shape() );
	lowShapeButton->set_parm_ptr( currGeom->get_lower_shape() );
	inlXRotButton->set_parm_ptr( currGeom->get_inl_x_rot() );
	scarfButton->set_parm_ptr( currGeom->get_inl_scarf() );



	if ( currGeom->get_inl_duct_flag() )
	{
		ui->ductOnButton->value(1);
		ui->ductOffButton->value(0);
	}
	else
	{
		ui->ductOnButton->value(0);
		ui->ductOffButton->value(1);
	}


	ui->UIWindow->show();

}




void EngineScreen::parm_changed( Parm* parm )
{
	if ( parm )
	{
	}
}

void EngineScreen::hide()
{
	engineUI->UIWindow->hide();
}

void EngineScreen::position( int x, int y )
{
	engineUI->UIWindow->position( x, y );
}

void EngineScreen::setTitle( const char* name )
{
	title = "ENGINE : ";
	title.concatenate( name );

	engineUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void EngineScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	engineUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void EngineScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	if ( w == engineUI->ductOnButton )
	{
		currGeom->set_inl_duct_flag(engineUI->ductOnButton->value());
		engineUI->ductOffButton->value( !engineUI->ductOnButton->value() );
		currGeom->parm_changed( currGeom->get_inl_duct_shape() );
		scriptMgr->addLine("engine_duct on");
	}
	else if ( w == engineUI->ductOffButton )
	{
		currGeom->set_inl_duct_flag(!engineUI->ductOffButton->value());
		engineUI->ductOnButton->value( !engineUI->ductOffButton->value() );
		currGeom->parm_changed( currGeom->get_inl_duct_shape() );
		scriptMgr->addLine("engine_duct off");
	}


	CompScreen::screenCB( w );

}


