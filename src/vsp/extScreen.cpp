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

#include "extScreen.h"
#include "ext_geom.h"
#include "materialMgr.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//==== Constructor =====//
ExtScreen::ExtScreen(ScreenMgr* mgr) : CompScreen( mgr )
{

	ExtUI* ui = extUI = new ExtUI();

	ui->UIWindow->position( 760, 30 );
	ui->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 10.0 );
	lengthInput  = new Input( this, ui->lengthInput );

	finessSlider = new Slider_adj_range(this, ui->finessSlider, ui->finessButtonL, ui->finessButtonR, 10.0 );
	finessInput  = new Input( this, ui->finessInput );
	cdSlider = new Slider_adj_range(this, ui->cdSlider, ui->cdButtonL, ui->cdButtonR, 10.0 );
	cdInput  = new Input( this, ui->cdInput );
	pylonHeightSlider = new Slider_adj_range(this, ui->pylonHeightSlider, ui->pylonHeightButtonL, ui->pylonHeightButtonR, 10.0 );
	pylonHeightInput  = new Input( this, ui->pylonHeightInput );
	pylonCdSlider = new Slider_adj_range(this, ui->cdPylonSlider, ui->cdPylonButtonL, ui->cdPylonButtonR, 10.0 );
	pylonCdInput  = new Input( this, ui->cdPylonInput );

	lengthButton = new ParmButton(this, ui->lengthButton );
	finessButton = new ParmButton(this, ui->finessButton );
	pylonHeightButton = new ParmButton(this, ui->pylonHeightButton );

	ui->typeChoice->callback( staticScreenCB, this );
	ui->pylonOnButton->callback( staticScreenCB, this );
	ui->pylonOffButton->callback( staticScreenCB, this );

	//==== Init General Comp Screen UI ====//
	CompScreen::init();


}

//==== Destructor =====//
ExtScreen::~ExtScreen()
{
}

void ExtScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (Ext_geom*)geomPtr;

	ExtUI* ui = extUI;

	//==== Ext Stores Stuff ====//
	lengthSlider->set_parm_ptr( currGeom->get_length() );
	lengthInput->set_parm_ptr( currGeom->get_length() );
	finessSlider->set_parm_ptr( currGeom->get_fine_ratio() );
	finessInput->set_parm_ptr( currGeom->get_fine_ratio() );
	cdSlider->set_parm_ptr( currGeom->get_drag() );
	cdInput->set_parm_ptr( currGeom->get_drag() );
	pylonHeightSlider->set_parm_ptr( currGeom->get_pylon_height() );
	pylonHeightInput->set_parm_ptr( currGeom->get_pylon_height() );
	pylonCdSlider->set_parm_ptr( currGeom->get_pylon_drag() );
	pylonCdInput->set_parm_ptr( currGeom->get_pylon_drag() );

	lengthButton->set_parm_ptr( currGeom->get_length() );
	finessButton->set_parm_ptr( currGeom->get_fine_ratio() );
	pylonHeightButton->set_parm_ptr( currGeom->get_pylon_height() );

	ui->typeChoice->value( currGeom->get_ext_type() );

	if ( currGeom->get_pylon_flag() )
	{
		ui->pylonOnButton->value(1);
		ui->pylonOffButton->value(0);
	}
	else
	{
		ui->pylonOnButton->value(0);
		ui->pylonOffButton->value(1);
	}

	ui->UIWindow->show();

}

void ExtScreen::hide()
{
	extUI->UIWindow->hide();
}

void ExtScreen::position( int x, int y )
{
	extUI->UIWindow->position( x, y );
}

void ExtScreen::setTitle( const char* name )
{
	title = "EXT STORE : ";
	title.concatenate( name );

	extUI->TitleBox->label( title );
}

void ExtScreen::parm_changed( Parm* parm )
{
}


//==== Close Callbacks =====//
void ExtScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	extUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void ExtScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	
	CompScreen::screenCB( w );


	ExtUI* ui = extUI;

	if ( w == ui->typeChoice )
	{
		int val = ui->typeChoice->value();
		s_type(ScriptMgr::GUI, val);
		/*
		currGeom->set_ext_type( ui->typeChoice->value() );
		currGeom->generate();
		*/
	}
	else if ( w == ui->pylonOnButton )
	{
		s_pylon(ScriptMgr::GUI, 1);
		/*
		currGeom->set_pylon_flag( 1 );
		ui->pylonOffButton->value(0);
		currGeom->generate();
		*/
	}
	else if ( w == ui->pylonOffButton )
	{
		s_pylon(ScriptMgr::GUI, 0);
		/*
		currGeom->set_pylon_flag( 0 );
		ui->pylonOnButton->value(0);
		currGeom->generate();
		*/
	}
}

void ExtScreen::s_type(int src, int val)
{
	currGeom->set_ext_type( val );
	currGeom->generate();

	if (src == ScriptMgr::GUI)
	{
		switch (val)
		{
			case BOMB_TYPE:
				scriptMgr->addLine("ext_design", "type", "bomb");
				break;
			case MISSLE_TYPE:
				scriptMgr->addLine("ext_design", "type", "missle");
				break;
			case TANK_TYPE:
				scriptMgr->addLine("ext_design", "type", "tank");
				break;
			case FIXED_TANK_TYPE:
				scriptMgr->addLine("ext_design", "type", "fixedtank");
				break;
		}
	}
}

void ExtScreen::s_pylon(int src, int on)
{
	currGeom->set_pylon_flag( on );
	currGeom->generate();
	if (src != ScriptMgr::SCRIPT)
	{
		extUI->pylonOnButton->value(on);
		extUI->pylonOffButton->value(!on);
	}
	if (src == ScriptMgr::GUI)
	{
		scriptMgr->addLine("ext_design", "pylon", on);
	}
}

