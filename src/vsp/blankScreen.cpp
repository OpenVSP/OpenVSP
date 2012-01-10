//****************************************************************************
//    
//   Blank Geom Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "blankScreen.h"
#include "geom.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//==== Constructor =====//
BlankScreen::BlankScreen(ScreenMgr* mgr) : VspScreen( mgr )
{

	BlankUI* ui = blankUI = new BlankUI();

	ui->UIWindow->position( 760, 30 );
	ui->UIWindow->callback( staticCloseCB, this );

	//==== Standard Geom Stuff ====//
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

	uSlider = new Slider(this, ui->uSlider );
	uSlider->setScriptText("gen setu", 0);
	vSlider = new Slider(this, ui->vSlider );
	vSlider->setScriptText("gen setv", 0);
	uInput = new Input( this, ui->uInput );
	vInput = new Input( this, ui->vInput );

	ui->relButton->callback( staticScreenCB, this );
	ui->absButton->callback( staticScreenCB, this );

	ui->nameInput->callback( staticScreenCB, this );

	ui->attachFixedButton->callback( staticScreenCB, this );
	ui->attachUVButton->callback( staticScreenCB, this );
	ui->attachMatrixButton->callback( staticScreenCB, this );

	ui->pointMassEnableButton->callback( staticScreenCB, this );
	massInput = new Input( this, ui->massInput );

	xLocButton = new ParmButton( this, ui->xLocButton );
	yLocButton = new ParmButton( this, ui->yLocButton );
	zLocButton = new ParmButton( this, ui->zLocButton );

	xRotButton = new ParmButton( this, ui->xRotButton );
	yRotButton = new ParmButton( this, ui->yRotButton );
	zRotButton = new ParmButton( this, ui->zRotButton );


	//script
	xLocSlider->setScriptText("xform xloc", 0);
	yLocSlider->setScriptText("xform yloc", 0);
	zLocSlider->setScriptText("xform zloc", 0);

	xRotSlider->setScriptText("xform xrot", 0);
	yRotSlider->setScriptText("xform yrot", 0);
	zRotSlider->setScriptText("xform zrot", 0);
}

//==== Destructor =====//
BlankScreen::~BlankScreen()
{
}

void BlankScreen::show(Geom* geomPtr)
{
	currGeom = (BlankGeom*)geomPtr;

	BlankUI* ui = blankUI;

	if ( geomPtr->getRelXFormFlag() )
	{
		xLocSlider->set_parm_ptr( &currGeom->xRelLoc );
		yLocSlider->set_parm_ptr( &currGeom->yRelLoc );
		zLocSlider->set_parm_ptr( &currGeom->zRelLoc );

		xLocInput->set_parm_ptr( &currGeom->xRelLoc );
		yLocInput->set_parm_ptr( &currGeom->yRelLoc );
		zLocInput->set_parm_ptr( &currGeom->zRelLoc );

		xRotSlider->set_parm_ptr( &currGeom->xRelRot );
		yRotSlider->set_parm_ptr( &currGeom->yRelRot );
		zRotSlider->set_parm_ptr( &currGeom->zRelRot );

		xRotInput->set_parm_ptr( &currGeom->xRelRot );
		yRotInput->set_parm_ptr( &currGeom->yRelRot );
		zRotInput->set_parm_ptr( &currGeom->zRelRot );

		xLocButton->set_parm_ptr( &currGeom->xRelLoc );
		yLocButton->set_parm_ptr( &currGeom->yRelLoc );
		zLocButton->set_parm_ptr( &currGeom->zRelLoc );
		xRotButton->set_parm_ptr( &currGeom->xRelRot );
		yRotButton->set_parm_ptr( &currGeom->yRelRot );
		zRotButton->set_parm_ptr( &currGeom->zRelRot );


		ui->relButton->value(1);
		ui->absButton->value(0);
	}
	else
	{
		xLocSlider->set_parm_ptr( &currGeom->xLoc );
		yLocSlider->set_parm_ptr( &currGeom->yLoc );
		zLocSlider->set_parm_ptr( &currGeom->zLoc );

		xLocInput->set_parm_ptr( &currGeom->xLoc );
		yLocInput->set_parm_ptr( &currGeom->yLoc );
		zLocInput->set_parm_ptr( &currGeom->zLoc );

		xRotSlider->set_parm_ptr( &currGeom->xRot );
		yRotSlider->set_parm_ptr( &currGeom->yRot );
		zRotSlider->set_parm_ptr( &currGeom->zRot );

		xRotInput->set_parm_ptr( &currGeom->xRot );
		yRotInput->set_parm_ptr( &currGeom->yRot );
		zRotInput->set_parm_ptr( &currGeom->zRot );

		xLocButton->set_parm_ptr( &currGeom->xLoc );
		yLocButton->set_parm_ptr( &currGeom->yLoc );
		zLocButton->set_parm_ptr( &currGeom->zLoc );
		xRotButton->set_parm_ptr( &currGeom->xRot );
		yRotButton->set_parm_ptr( &currGeom->yRot );
		zRotButton->set_parm_ptr( &currGeom->zRot );


		ui->relButton->value(0);
		ui->absButton->value(1);
	}

	setTitle( currGeom->getName().get_char_star() );
	ui->nameInput->value( currGeom->getName().get_char_star() );

	uSlider->set_parm_ptr( &currGeom->uAttach );
	vSlider->set_parm_ptr( &currGeom->vAttach );
	uInput->set_parm_ptr( &currGeom->uAttach );
	vInput->set_parm_ptr( &currGeom->vAttach );

	ui->attachFixedButton->value(0);
	ui->attachUVButton->value(0);
	ui->attachMatrixButton->value(0);

	if ( currGeom->getPosAttachFlag() == POS_ATTACH_FIXED )
		ui->attachFixedButton->value(1);
	else if ( currGeom->getPosAttachFlag() == POS_ATTACH_UV )
		ui->attachUVButton->value(1);
	else if ( currGeom->getPosAttachFlag() == POS_ATTACH_MATRIX )
		ui->attachMatrixButton->value(1);

	ui->pointMassEnableButton->value( currGeom->getPointMassFlag() );
	massInput->set_parm_ptr( &currGeom->pointMass );

	ui->UIWindow->show();

}

void BlankScreen::hide()
{
	blankUI->UIWindow->hide();
}

void BlankScreen::position( int x, int y )
{
	blankUI->UIWindow->position( x, y );
}

void BlankScreen::setTitle( const char* name )
{
	title = "BLANK GEOM : ";
	title.concatenate( name );

	blankUI->TitleBox->label( title );
}

void BlankScreen::parm_changed( Parm* parm )
{
}

//==== Close Callbacks =====//
void BlankScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	blankUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void BlankScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	BlankUI* ui = blankUI;

	if ( w == ui->absButton  )
	{
		if ( ui->absButton->value() )
		{
			ui->relButton->value(0);
			currGeom->setRelXFormFlag( 0 );
			scriptMgr->addLine("xform position abs");

		}
		else
		{
			ui->relButton->value(1);
			currGeom->setRelXFormFlag( 1 );
			scriptMgr->addLine("xform position rel");
		}
	}
	else if ( w == ui->relButton )
	{
		if ( ui->relButton->value() )
		{
			ui->absButton->value(0);
			currGeom->setRelXFormFlag( 1 );
			scriptMgr->addLine("xform position rel");
		}
		else
		{
			ui->absButton->value(1);
			currGeom->setRelXFormFlag( 0 );
			scriptMgr->addLine("xform position abs");
		}
	}
	else if ( w == ui->nameInput )
	{
		currGeom->setName( Stringc(ui->nameInput->value()) );

		//==== Show as Window Label ====//
		setTitle( currGeom->getName().get_char_star() );
		screenMgrPtr->update( GEOM_SCREEN );
		scriptMgr->addLine("gen name", currGeom->getName());
	}
	else if ( w == ui->attachFixedButton  )
	{
		if ( ui->attachFixedButton->value() )
			currGeom->setPosAttachFlag( POS_ATTACH_FIXED );
		else
			currGeom->setPosAttachFlag( POS_ATTACH_NONE );
		ui->attachUVButton->value(0);
		ui->attachMatrixButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );

		scriptMgr->addLine("gen attach fixed");
	}
	else if ( w == ui->attachUVButton )
	{
		if ( ui->attachUVButton->value() )
			currGeom->setPosAttachFlag( POS_ATTACH_UV );
		else
			currGeom->setPosAttachFlag( POS_ATTACH_NONE );
		ui->attachFixedButton->value(0);
		ui->attachMatrixButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );
		scriptMgr->addLine("gen attach uv");
	}
	else if ( w == ui->attachMatrixButton )
	{
		if ( ui->attachMatrixButton->value() )
			currGeom->setPosAttachFlag( POS_ATTACH_MATRIX );
		else
			currGeom->setPosAttachFlag( POS_ATTACH_NONE );

		ui->attachFixedButton->value(0);
		ui->attachUVButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );
		scriptMgr->addLine("gen attach matrix");
	}	
	else if ( w == ui->pointMassEnableButton )
	{
		currGeom->setPointMassFlag( ui->pointMassEnableButton->value() );
	}

}


