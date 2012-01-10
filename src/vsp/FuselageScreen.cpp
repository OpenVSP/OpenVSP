//****************************************************************************
//    
//   Fuselage Screen
//   
//   J.R. Gloudemans - 9/22/09
//****************************************************************************

#include "FuselageScreen.h"
#include "FuselageGeom.h"
#include "materialMgr.h"
#include "screenMgr.h"
#include "curveEditScreen.h"
#include "scriptMgr.h"


//==== Constructor =====//
FuselageScreen::FuselageScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	jpgImgData.data = 0;

	FuselageUI* ui = fuseUI = new FuselageUI();

	ui->UIWindow->position( 760, 30 );
	ui->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
    #include "CommonParmButtonGui.h"

	xsHeightSlider = new Slider_adj_range(this, ui->heightSlider, ui->heightButtonL, ui->heightButtonR, 10.0 );
	xsHeightInput  = new Input( this, ui->heightInput );

	xsWidthSlider = new Slider_adj_range(this, ui->widthSlider, ui->widthButtonL, ui->widthButtonR, 10.0 );
	xsWidthInput  = new Input( this, ui->widthInput );

	xsYOffsetSlider = new Slider_adj_range(this, ui->yOffsetSlider, ui->yOffsetButtonL, ui->yOffsetButtonR, 1.0 );
	xsYOffsetInput  = new Input( this, ui->yOffsetInput );

	xsZOffsetSlider = new Slider_adj_range(this, ui->zOffsetSlider, ui->zOffsetButtonL, ui->zOffsetButtonR, 1.0 );
	xsZOffsetInput  = new Input( this, ui->zOffsetInput );

	xsWidthOffsetSlider = new Slider_adj_range(this, ui->widthOffsetSlider, ui->widthOffsetButtonL, ui->widthOffsetButtonR, 1.0 );
	xsWidthOffsetInput  = new Input( this, ui->widthOffsetInput );

	xsCornerRadSlider = new Slider_adj_range(this, ui->cornerRadSlider, ui->cornerRadButtonL, ui->cornerRadButtonR, 1.0 );
	xsCornerRadInput  = new Input( this, ui->cornerRadInput );

	xsTopTanAngSlider = new Slider_adj_range(this, ui->topTanSlider, ui->topTanButtonL, ui->topTanButtonR, 90.0 );
	xsTopTanAngInput  = new Input( this, ui->topTanInput );

	xsBotTanAngSlider = new Slider_adj_range(this, ui->botTanSlider, ui->botTanButtonL, ui->botTanButtonR, 90.0 );
	xsBotTanAngInput  = new Input( this, ui->botTanInput );

	xsTanStrSlider = new Slider(this, ui->tanStrSlider);
	xsTanStrInput  = new Input( this, ui->tanStrInput );

	xsLocSlider = new Slider(this, ui->locationSlider);
	xsLocInput  = new Input( this, ui->locationInput );

	ui->xsTypeChoice->callback( staticScreenCB, this );
	ui->tanStrChoice->callback( staticScreenCB, this );
	ui->zoomRoller->callback( staticScreenCB, this );
	ui->backgroundButton->callback( staticScreenCB, this );
	ui->addButton->callback( staticScreenCB, this );
	ui->cutButton->callback( staticScreenCB, this );
	ui->copyButton->callback( staticScreenCB, this );
	ui->pasteButton->callback( staticScreenCB, this );
	ui->xsecIDCounter->callback( staticScreenCB, this );
	ui->tanXsecIDCounter->callback( staticScreenCB, this );
	ui->pointSpacingChoice->callback( staticScreenCB, this );
	ui->editButton->callback( staticScreenCB, this );
	ui->locationAbsInput->callback( staticScreenCB, this );

	//==== Profile Stuff =====//
	topAngSlider = new Slider(this, ui->topAngSlider );
	topAngInput  = new Input( this, ui->topAngInput );
	topStr1Slider = new Slider(this, ui->topStr1Slider );
	topStr1Input  = new Input( this, ui->topStr1Input );
	topStr2Slider = new Slider(this, ui->topStr2Slider );
	topStr2Input  = new Input( this, ui->topStr2Input );

	botAngSlider = new Slider(this, ui->botAngSlider );
	botAngInput  = new Input( this, ui->botAngInput );
	botStr1Slider = new Slider(this, ui->botStr1Slider );
	botStr1Input  = new Input( this, ui->botStr1Input );
	botStr2Slider = new Slider(this, ui->botStr2Slider );
	botStr2Input  = new Input( this, ui->botStr2Input );

	leftAngSlider = new Slider(this, ui->leftAngSlider );
	leftAngInput  = new Input( this, ui->leftAngInput );
	leftStr1Slider = new Slider(this, ui->leftStr1Slider );
	leftStr1Input  = new Input( this, ui->leftStr1Input );
	leftStr2Slider = new Slider(this, ui->leftStr2Slider );
	leftStr2Input  = new Input( this, ui->leftStr2Input );

	rightAngSlider = new Slider(this, ui->rightAngSlider );
	rightAngInput  = new Input( this, ui->rightAngInput );
	rightStr1Slider = new Slider(this, ui->rightStr1Slider );
	rightStr1Input  = new Input( this, ui->rightStr1Input );
	rightStr2Slider = new Slider(this, ui->rightStr2Slider );
	rightStr2Input  = new Input( this, ui->rightStr2Input );

	ui->topSymButton->callback( staticScreenCB, this );
	ui->sideSymButton->callback( staticScreenCB, this );

	//==== Shape Stuff =====//
	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 10 );
	lengthInput  = new Input( this, ui->lengthInput );

	xsHeightButton = new ParmButton( this, ui->heightButton );
	xsWidthButton = new ParmButton( this, ui->widthButton );
	xsYOffsetButton = new ParmButton( this, ui->yOffsetButton );
	xsZOffsetButton = new ParmButton( this, ui->zOffsetButton );
	xsWidthOffsetButton = new ParmButton( this, ui->widthOffsetButton);
	xsCornerRadButton = new ParmButton( this, ui->cornerRadButton );
	xsTopTanAngButton = new ParmButton( this, ui->topTanButton );
	xsBotTanAngButton = new ParmButton( this, ui->botTanButton );
	xsLocButton = new ParmButton( this, ui->locationButton );

	lengthButton = new ParmButton( this, ui->lengthButton );

	topAngButton = new ParmButton( this, ui->topAngButton );
	topStr1Button = new ParmButton( this, ui->topStr1Button );
	topStr2Button = new ParmButton( this, ui->topStr2Button );

	botAngButton = new ParmButton( this, ui->botAngButton );
	botStr1Button = new ParmButton( this, ui->botStr1Button );
	botStr2Button = new ParmButton( this, ui->botStr2Button );

	leftAngButton = new ParmButton( this, ui->leftAngButton );
	leftStr1Button = new ParmButton( this, ui->leftStr1Button );
	leftStr2Button = new ParmButton( this, ui->leftStr2Button );

	rightAngButton = new ParmButton( this, ui->rightAngButton );
	rightStr1Button = new ParmButton( this, ui->rightStr1Button );
	rightStr2Button = new ParmButton( this, ui->rightStr2Button );

	//==== Profile & XSec Interp Stuff ====//
	ui->numInterp1Counter->callback( staticScreenCB, this );
	ui->numInterp2Counter->callback( staticScreenCB, this );
	ui->decNumAllButton->callback( staticScreenCB, this );
	ui->incNumAllButton->callback( staticScreenCB, this );

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->glWindow;
	ui->glWindow->begin();
	glWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->glWindow->end();

	jpgImgData.data = 0;

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();



}

//==== Destructor =====//
FuselageScreen::~FuselageScreen()
{
}

void FuselageScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (FuselageGeom*)geomPtr;
	curr_fxsec = currGeom->get_curr_xsec();

	FuselageUI* ui = fuseUI;

	setTitle( currGeom->getName().get_char_star() );

	//==== Fuse Specific UI ====//
	ui->xsTypeChoice->value( curr_fxsec->get_type() );
	ui->pointSpacingChoice->value( curr_fxsec->get_pnt_space_type() );

	xsHeightSlider->set_parm_ptr( curr_fxsec->get_height() );
	xsHeightInput->set_parm_ptr( curr_fxsec->get_height() );

	xsWidthSlider->set_parm_ptr( curr_fxsec->get_width() );
	xsWidthInput->set_parm_ptr( curr_fxsec->get_width() );

	xsYOffsetSlider->set_parm_ptr( curr_fxsec->get_y_offset() );
	xsYOffsetInput->set_parm_ptr(  curr_fxsec->get_y_offset() );

	xsZOffsetSlider->set_parm_ptr( curr_fxsec->get_z_offset() );
	xsZOffsetInput->set_parm_ptr(  curr_fxsec->get_z_offset() );

	xsWidthOffsetSlider->set_parm_ptr( curr_fxsec->get_max_width_loc() );
	xsWidthOffsetInput->set_parm_ptr(  curr_fxsec->get_max_width_loc() );

	xsCornerRadSlider->set_parm_ptr( curr_fxsec->get_corner_rad() );
	xsCornerRadInput->set_parm_ptr(  curr_fxsec->get_corner_rad()  );

	xsTopTanAngSlider->set_parm_ptr( curr_fxsec->get_top_tan_angle() );
	xsTopTanAngInput->set_parm_ptr(  curr_fxsec->get_top_tan_angle()  );

	xsBotTanAngSlider->set_parm_ptr( curr_fxsec->get_bot_tan_angle() );
	xsBotTanAngInput->set_parm_ptr(  curr_fxsec->get_bot_tan_angle()  );

	xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
	xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );

	xsLocSlider->set_parm_ptr( curr_fxsec->get_location() );
	xsLocInput->set_parm_ptr( curr_fxsec->get_location() );

	ui->zoomRoller->value( curr_fxsec->getRollerScaleFactor() );

	ui->xsecIDCounter->bounds(0, currGeom->get_num_xsecs()-1);
	ui->xsecIDCounter->value( currGeom->get_curr_xsec_num() );

	ui->tanXsecIDCounter->bounds(0, currGeom->get_num_xsecs()-1);
	ui->tanXsecIDCounter->value( currGeom->get_curr_xsec_num() );


	glWin->setDrawBase( curr_fxsec );
	glWin->redraw();

	lengthSlider->set_parm_ptr( currGeom->get_length() );
	lengthInput->set_parm_ptr(  currGeom->get_length() );

	//==== Profile Stuff ====//
	topAngSlider->set_parm_ptr( curr_fxsec->getTopTanAng() );
	topAngInput->set_parm_ptr( curr_fxsec->getTopTanAng() );
	topStr1Slider->set_parm_ptr( curr_fxsec->getTopTanStr1() );
	topStr1Input->set_parm_ptr( curr_fxsec->getTopTanStr1() );
	topStr2Slider->set_parm_ptr( curr_fxsec->getTopTanStr2() );
	topStr2Input->set_parm_ptr( curr_fxsec->getTopTanStr2() );

	botAngSlider->set_parm_ptr( curr_fxsec->getBotTanAng() );
	botAngInput->set_parm_ptr( curr_fxsec->getBotTanAng() );
	botStr1Slider->set_parm_ptr( curr_fxsec->getBotTanStr1() );
	botStr1Input->set_parm_ptr( curr_fxsec->getBotTanStr1() );
	botStr2Slider->set_parm_ptr( curr_fxsec->getBotTanStr2() );
	botStr2Input->set_parm_ptr( curr_fxsec->getBotTanStr2() );

	leftAngSlider->set_parm_ptr( curr_fxsec->getLeftTanAng() );
	leftAngInput->set_parm_ptr( curr_fxsec->getLeftTanAng() );
	leftStr1Slider->set_parm_ptr( curr_fxsec->getLeftTanStr1() );
	leftStr1Input->set_parm_ptr( curr_fxsec->getLeftTanStr1() );
	leftStr2Slider->set_parm_ptr( curr_fxsec->getLeftTanStr2() );
	leftStr2Input->set_parm_ptr( curr_fxsec->getLeftTanStr2() );

	rightAngSlider->set_parm_ptr( curr_fxsec->getRightTanAng() );
	rightAngInput->set_parm_ptr( curr_fxsec->getRightTanAng() );
	rightStr1Slider->set_parm_ptr( curr_fxsec->getRightTanStr1() );
	rightStr1Input->set_parm_ptr( curr_fxsec->getRightTanStr1() );
	rightStr2Slider->set_parm_ptr( curr_fxsec->getRightTanStr2() );
	rightStr2Input->set_parm_ptr( curr_fxsec->getRightTanStr2() );

	fuseUI->numInterp1Counter->value( curr_fxsec->getNumSectInterp1() );
	fuseUI->numInterp2Counter->value( curr_fxsec->getNumSectInterp2() );

	fuseUI->topSymButton->value( curr_fxsec->get_top_sym() );
	fuseUI->sideSymButton->value( curr_fxsec->get_side_sym() );

	setActiveShapeGUI();

	ui->tanXsecIDCounter->bounds(0, currGeom->get_num_xsecs()-1);
	ui->tanXsecIDCounter->value( currGeom->get_curr_xsec_num() );

	Fl_Menu_* mw = (Fl_Menu_*)ui->tanStrChoice;
	const Fl_Menu_Item* m = mw->mvalue();
	if ( m == ui->allTanStr )
	{
		curr_fxsec->setAllTanStrFlag(1);
		xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
		xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
	}
	else if ( m == ui->topTanStr )
	{
		curr_fxsec->setAllTanStrFlag(0);
		xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
		xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
	}
	else if ( m == ui->upCornerTanStr )
	{
		curr_fxsec->setAllTanStrFlag(0);
		xsTanStrSlider->set_parm_ptr( curr_fxsec->get_upp_str() );
		xsTanStrInput->set_parm_ptr(  curr_fxsec->get_upp_str() );
	}
	else if ( m == ui->lowCornerTanStr  )
	{
		curr_fxsec->setAllTanStrFlag(0);
		xsTanStrSlider->set_parm_ptr( curr_fxsec->get_low_str() );
		xsTanStrInput->set_parm_ptr(  curr_fxsec->get_low_str() );
	}
	else if ( m == ui->botTanStr )
	{
		curr_fxsec->setAllTanStrFlag(0);
		xsTanStrSlider->set_parm_ptr( curr_fxsec->get_bot_str() );
		xsTanStrInput->set_parm_ptr(  curr_fxsec->get_bot_str() );
	}

	xsHeightButton->set_parm_ptr( curr_fxsec->get_height() );
	xsWidthButton->set_parm_ptr( curr_fxsec->get_width() );
	xsYOffsetButton->set_parm_ptr( curr_fxsec->get_y_offset() );
	xsZOffsetButton->set_parm_ptr( curr_fxsec->get_z_offset() );
	xsWidthOffsetButton->set_parm_ptr( curr_fxsec->get_max_width_loc() );
	xsCornerRadButton->set_parm_ptr( curr_fxsec->get_corner_rad() );
	xsTopTanAngButton->set_parm_ptr( curr_fxsec->get_top_tan_angle() );
	xsBotTanAngButton->set_parm_ptr( curr_fxsec->get_bot_tan_angle() );
	xsLocButton->set_parm_ptr( curr_fxsec->get_location() );

	lengthButton->set_parm_ptr( currGeom->get_length() );

	topAngButton->set_parm_ptr( curr_fxsec->getTopTanAng() );
	topStr1Button->set_parm_ptr( curr_fxsec->getTopTanStr1() );
	topStr2Button->set_parm_ptr( curr_fxsec->getTopTanStr2() );

	botAngButton->set_parm_ptr( curr_fxsec->getBotTanAng() );
	botStr1Button->set_parm_ptr( curr_fxsec->getBotTanStr1() );
	botStr2Button->set_parm_ptr( curr_fxsec->getBotTanStr2() );

	leftAngButton->set_parm_ptr( curr_fxsec->getLeftTanAng() );
	leftStr1Button->set_parm_ptr( curr_fxsec->getLeftTanStr1() );
	leftStr2Button->set_parm_ptr( curr_fxsec->getLeftTanStr2() );

	rightAngButton->set_parm_ptr( curr_fxsec->getRightTanAng() );
	rightStr1Button->set_parm_ptr( curr_fxsec->getRightTanStr1() );
	rightStr2Button->set_parm_ptr( curr_fxsec->getRightTanStr2() );

	updateAbsXSecLocVal();

	ui->UIWindow->show();

	if ( curr_fxsec->get_type() == FXS_EDIT_CRV )
	{
		screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
		screenMgrPtr->getCurveEditScreen()->show();		
	}
	else
	{
		screenMgrPtr->getCurveEditScreen()->hide();	
	}

}

void FuselageScreen::updateAbsXSecLocVal()
{
	char str[255];
	sprintf( str, "%0.4f", curr_fxsec->get_loc_on_spine() * currGeom->get_length()->get() );
	fuseUI->locationAbsInput->value(str);
}

void FuselageScreen::setActiveShapeGUI()
{
	botAngSlider->activate();	
	botAngInput->activate();	
	botStr1Slider->activate();	
	botStr1Input->activate();	
	botStr2Slider->activate();	
	botStr2Input->activate();	
	if ( curr_fxsec->get_top_sym() )
	{
		botAngSlider->deactivate();	
		botAngInput->deactivate();	
		botStr1Slider->deactivate();	
		botStr1Input->deactivate();	
		botStr2Slider->deactivate();	
		botStr2Input->deactivate();	
	}

	rightAngSlider->activate();	
	rightAngInput->activate();	
	rightStr1Slider->activate();	
	rightStr1Input->activate();	
	rightStr2Slider->activate();	
	rightStr2Input->activate();	
	if ( curr_fxsec->get_side_sym() )
	{
		rightAngSlider->deactivate();	
		rightAngInput->deactivate();	
		rightStr1Slider->deactivate();	
		rightStr1Input->deactivate();	
		rightStr2Slider->deactivate();	
		rightStr2Input->deactivate();	
	}

}

void FuselageScreen::hide()
{

	fuseUI->UIWindow->hide();
	screenMgrPtr->getCurveEditScreen()->hide();	

}

void FuselageScreen::position( int x, int y )
{
	fuseUI->UIWindow->position( x, y );
}

void FuselageScreen::setTitle( const char* name )
{
	title = "FUSELAGE : ";
	title.concatenate( name );

	fuseUI->TitleBox->label( title );
}

void FuselageScreen::parm_changed( Parm* parm )
{
	if ( parm )
	{
		updateAbsXSecLocVal();
		if ( parm->get_update_grp() == UPD_FUSE_XSEC )
		{
			glWin->redraw();
		}
	}
}

//==== Close Callbacks =====//
void FuselageScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	fuseUI->UIWindow->hide();
	screenMgrPtr->getCurveEditScreen()->hide();	
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void FuselageScreen::screenCB( Fl_Widget* w)
{
	CompScreen::screenCB( w );

	if ( !currGeom )
		return;

	FuselageUI* ui = fuseUI;

	if ( w == ui->xsTypeChoice  )
	{
		int val = ui->xsTypeChoice->value();
		s_type(ScriptMgr::GUI,  val);
	}
	else if ( w == ui->editButton )
	{
		if ( curr_fxsec->get_type() == FXS_EDIT_CRV )
		{
			screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
			screenMgrPtr->getCurveEditScreen()->show();	
		}
		else if ( curr_fxsec->get_type() == FXS_FROM_FILE )
		{
			curr_fxsec->set_type( FXS_EDIT_CRV );			// Hack to bring up file browser
			s_type(ScriptMgr::GUI, FXS_FROM_FILE );
		}

	}
	else if ( w == ui->pointSpacingChoice )
	{
		int val = ui->pointSpacingChoice->value();	
		currGeom->set_pnt_space_type( val );

		switch (val)
		{
			case PNT_SPACE_PER_XSEC:
				scriptMgr->addLine("gen pointspacing xsec");
				break;
			case PNT_SPACE_FIXED:
				scriptMgr->addLine("gen pointspacing fixed");
				break;
			case PNT_SPACE_UNIFORM:
				scriptMgr->addLine("gen pointspacing uniform");
				break;
		}
	}
	else if ( w == ui->xsecIDCounter  )
	{
		int id = (int)ui->xsecIDCounter->value();
		s_set_id(ScriptMgr::GUI, id);
		s_type( ScriptMgr::GUI,  curr_fxsec->get_type() );
	}
	else if ( w == ui->tanXsecIDCounter  )
	{
		int id = (int)ui->tanXsecIDCounter->value();
		s_set_id(ScriptMgr::GUI, id);
	}
	else if ( w == ui->addButton )
	{
		s_xsec_add(ScriptMgr::GUI);
	}
	else if ( w == ui->cutButton )
	{
		s_xsec_cut(ScriptMgr::GUI);
	}
	else if ( w == ui->copyButton )
	{
		s_xsec_copy(ScriptMgr::GUI);
	}
	else if ( w == ui->pasteButton )
	{
		s_xsec_paste(ScriptMgr::GUI);
	}
	else if ( w == ui->backgroundButton )
	{
		if ( ui->backgroundButton->value() )
		{

//			char *newfile = fl_file_chooser("Read Background File?", "*.jpg", "");
			char *newfile = screenMgrPtr->FileChooser("Read Background File?", "*.jpg");
			if ( newfile != NULL )
			{
				deleteJPEG( jpgImgData );
				jpgImgData = loadJPEG(newfile);
				glWin->setBackImg( jpgImgData );
			}
			glWin->setDrawBackImgFlag( 1 );
		}
		else
		{
			glWin->setDrawBackImgFlag( 0 );
		}
		glWin->redraw();
		
	}
	else if ( w == ui->zoomRoller )
	{
		curr_fxsec->setDrawScaleFactor( ui->zoomRoller->value() );
		glWin->redraw();
	}
	else if ( w == ui->tanStrChoice  )
	{
		Fl_Menu_* mw = (Fl_Menu_*)w;
		const Fl_Menu_Item* m = mw->mvalue();
		if ( m == ui->allTanStr )
		{
			curr_fxsec->setAllTanStrFlag( 1 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
		}
		else if ( m == ui->topTanStr )
		{
			curr_fxsec->setAllTanStrFlag( 0 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
		}
		else if ( m == ui->upCornerTanStr )
		{
			curr_fxsec->setAllTanStrFlag( 0 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_upp_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_upp_str() );
		}
		else if ( m == ui->lowCornerTanStr  )
		{
			curr_fxsec->setAllTanStrFlag( 0 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_low_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_low_str() );
		}
		else if ( m == ui->botTanStr )
		{
			curr_fxsec->setAllTanStrFlag( 0 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_bot_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_bot_str() );
		}
	}
	else if ( w == ui->numInterp1Counter )
	{
		int val = (int)ui->numInterp1Counter->value();
		s_xsec_num_interp1(ScriptMgr::GUI, val );
	}
	else if ( w == ui->numInterp2Counter )
	{
		int val = (int)ui->numInterp2Counter->value();
		s_xsec_num_interp2(ScriptMgr::GUI, val );
	}
	else if ( w == ui->decNumAllButton )
	{
		s_xsec_dec_num_all(ScriptMgr::GUI);
	}
	else if ( w == ui->incNumAllButton )
	{
		s_xsec_inc_num_all(ScriptMgr::GUI);
	}
	else if ( w == ui->topSymButton  )
	{
		curr_fxsec->set_top_sym( ui->topSymButton->value() );
		setActiveShapeGUI();
	}
	else if ( w == ui->sideSymButton  )
	{
		curr_fxsec->set_side_sym( ui->sideSymButton->value() );
		setActiveShapeGUI();
	}
	else if ( w == ui->locationAbsInput )
	{
		double val = atof( ui->locationAbsInput->value() );
		double fracLoc = val/currGeom->get_length()->get();
		curr_fxsec->get_location()->set( fracLoc );
		currGeom->generate();
		glWin->redraw();
		updateAbsXSecLocVal();
	}


}


void FuselageScreen::s_set_id(int src, int id) 
{
	currGeom->set_curr_xsec_num(id);
	curr_fxsec = currGeom->get_curr_xsec();
	currGeom->generate();

	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec", "id", id);
	}
}


void FuselageScreen::s_xsec_add(int src) {
	curr_fxsec = currGeom->add_xsec(curr_fxsec);
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec add");
	}
}

void FuselageScreen::s_xsec_cut(int src) {
		curr_fxsec = currGeom->delete_xsec( curr_fxsec );
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec cut");
	}
}

void FuselageScreen::s_xsec_copy(int src) {
	currGeom->copy_xsec(curr_fxsec);
	if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec copy");
}

void FuselageScreen::s_xsec_paste(int src) {
	currGeom->paste_xsec(curr_fxsec);
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec paste");
	}
}

void FuselageScreen::s_type(int src, int val)
{
	int old_type = curr_fxsec->get_type();
	curr_fxsec->set_type( val );

	if (src == ScriptMgr::SCRIPT)
	{
		if ( curr_fxsec->get_type() == FXS_RND_BOX )
		{
			curr_fxsec->setAllTanStrFlag( 1 );
		}

		curr_fxsec->regenerate();
		currGeom->generate();
	}
	else
	{
		FuselageUI* ui = fuseUI;

		if ( curr_fxsec->get_type() == FXS_GENERAL )
		{
			ui->tanStrChoice->activate();
		}
		else if ( curr_fxsec->get_type() == FXS_RND_BOX )
		{
			ui->tanStrChoice->value(0);	
			curr_fxsec->setAllTanStrFlag( 1 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
		}
		else if ( curr_fxsec->get_type() == FXS_FROM_FILE  )
		{
			if ( old_type != FXS_FROM_FILE )
			{
//				char *newfile = fl_file_chooser("Read Fuse XSec File?", "*.fxs", "");
				char *newfile = screenMgrPtr->FileChooser("Read Fuse XSec File?", "*.fxs");

				int success = 0;
				if ( newfile != NULL )
				{
					success = curr_fxsec->read_xsec_file( Stringc(newfile) );
					glWin->redraw();
				}
				if ( !success )
				{
					ui->xsTypeChoice->value( FXS_GENERAL );
					curr_fxsec->set_type( FXS_GENERAL );
					ui->tanStrChoice->activate();
				}
			}
		}
		else if ( curr_fxsec->get_type() == FXS_EDIT_CRV  )
		{
			screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
			screenMgrPtr->getCurveEditScreen()->show();			
		}
		else
		{
			ui->tanStrChoice->deactivate();
		}

		if ( curr_fxsec->get_type() != FXS_EDIT_CRV )
			screenMgrPtr->getCurveEditScreen()->hide();			


		curr_fxsec->regenerate();
		currGeom->generate();
		glWin->redraw();

		if (src == ScriptMgr::GUI)
		{
			switch (val)
			{
				case FXS_POINT:
					scriptMgr->addLine("fuselage_xsec type point");
					break;
				case FXS_CIRCLE:
					scriptMgr->addLine("fuselage_xsec type circle");
					break;
				case FXS_ELLIPSE:
					scriptMgr->addLine("fuselage_xsec type ellipse");
					break;
				case FXS_RND_BOX:
					scriptMgr->addLine("fuselage_xsec type roundbox");
					break;
				case FXS_GENERAL:
					scriptMgr->addLine("fuselage_xsec type general");
					break;
				case FXS_FROM_FILE:
					scriptMgr->addLine("fuselage_xsec type file");
					break;
				case FXS_EDIT_CRV:
					scriptMgr->addLine("fuselage_xsec type edit");
					break;
			}
		}
	}
}

void FuselageScreen::s_xsec_num_interp1(int src, int val)
{
	curr_fxsec->setNumSectInterp1( val );
	currGeom->updateNumInter();
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec", "xsec_num_interp1", val);
	}
}

void FuselageScreen::s_xsec_num_interp2(int src, int val)
{
	curr_fxsec->setNumSectInterp2( val );
	currGeom->updateNumInter();
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_xsec", "xsec_num_interp2", val);
	}
}

void FuselageScreen::s_xsec_dec_num_all(int src)
{
	currGeom->incNumInterpAll( -1 );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_shape", "xsec_dec_num_all");
	}
}

void FuselageScreen::s_xsec_inc_num_all(int src)
{
	currGeom->incNumInterpAll( 1 );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuselage_shape", "xsec_inc_num_all");
	}
}


