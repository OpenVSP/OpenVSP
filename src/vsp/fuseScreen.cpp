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

#include "fuseScreen.h"
#include "fuse_geom.h"
#include "fuseFlScreen.h"
#include "materialMgr.h"
#include "screenMgr.h"
#include "curveEditScreen.h"
#include "scriptMgr.h"


//==== Constructor =====//
FuseScreen::FuseScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	jpgImgData.data = 0;

	FuseUI* ui = fuseUI = new FuseUI();

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

	//==== Profile Stuff =====//
	profileTanAngSlider = new Slider_adj_range(this, ui->profileAngWtSlider, ui->profileAngWtButtonL, ui->profileAngWtButtonR, 2.0 );
	profileTanAngInput  = new Input( this, ui->profileAngWtInput );

	profileTanStr1Slider = new Slider_adj_range(this, ui->profileTanStr1Slider, ui->profileTanStr1ButtonL, ui->profileTanStr1ButtonR, 1.0 );
	profileTanStr1Input  = new Input( this, ui->profileTanStr1Input );

	profileTanStr2Slider = new Slider_adj_range(this, ui->profileTanStr2Slider, ui->profileTanStr2ButtonL, ui->profileTanStr2ButtonR, 1.0 );
	profileTanStr2Input  = new Input( this, ui->profileTanStr2Input );

	ui->noseRhoOnButton->callback( staticScreenCB, this );
	ui->aftRhoOnButton->callback(  staticScreenCB, this );

	//==== Shape Stuff =====//
	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 20.0 );
	lengthInput  = new Input( this, ui->lengthInput );
	camberSlider = new Slider_adj_range(this, ui->camberSlider, ui->camberButtonL, ui->camberButtonR, 0.1 );
	camberInput  = new Input( this, ui->camberInput );
	camberLocSlider = new Slider_adj_range(this, ui->camberLocSlider, ui->camberLocButtonL, ui->camberLocButtonR, 0.4 );
	camberLocInput  = new Input( this, ui->camberLocInput );
	aftOffsetSlider = new Slider_adj_range(this, ui->aftOffsetSlider, ui->aftOffsetButtonL, ui->aftOffsetButtonR, 0.1 );
	aftOffsetInput  = new Input( this, ui->aftOffsetInput );
	noseAngleSlider = new Slider_adj_range(this, ui->noseAngleSlider, ui->noseAngleButtonL, ui->noseAngleButtonR, 10.0 );
	noseAngleInput  = new Input( this, ui->noseAngleInput );
	noseStrSlider = new Slider_adj_range(this, ui->noseStrSlider, ui->noseStrButtonL, ui->noseStrButtonR, 10.0 );
	noseStrInput  = new Input( this, ui->noseStrInput );
	noseRhoSlider = new Slider_adj_range(this, ui->noseRhoSlider, ui->noseRhoButtonL, ui->noseRhoButtonR, 10.0 );
	noseRhoInput  = new Input( this, ui->noseRhoInput );
	aftRhoSlider = new Slider_adj_range(this, ui->aftRhoSlider, ui->aftRhoButtonL, ui->aftRhoButtonR, 10.0 );
	aftRhoInput  = new Input( this, ui->aftRhoInput );

	//==== Iml Stuff ====//
	topThickSlider = new Slider_adj_range(this, ui->topThickSlider, ui->topThickButtonL, ui->topThickButtonR, 10.0 );
	topThickInput  = new Input( this, ui->topThickInput );
	botThickSlider = new Slider_adj_range(this, ui->botThickSlider, ui->botThickButtonL, ui->botThickButtonR, 10.0 );
	botThickInput  = new Input( this, ui->botThickInput );
	sideThickSlider = new Slider_adj_range(this, ui->sideThickSlider, ui->sideThickButtonL, ui->sideThickButtonR, 10.0 );
	sideThickInput  = new Input( this, ui->sideThickInput );

	ui->imlMaterialChoice->clear();
	for (  int i = 0 ; i < matMgrPtr->getNumMaterial() ; i++ )
	{
		Material* mat = matMgrPtr->getMaterial(i);
		if ( mat )
			ui->imlMaterialChoice->add( mat->name );
	}
	ui->imlMaterialChoice->callback( staticScreenCB, this );

	ui->imlRedSlider->callback( staticScreenCB, this );
	ui->imlGreenSlider->callback( staticScreenCB, this );
	ui->imlBlueSlider->callback( staticScreenCB, this );

	ui->imlRedButton->callback( staticScreenCB, this );
	ui->imlGreenButton->callback( staticScreenCB, this );
	ui->imlBlueButton->callback( staticScreenCB, this );
	ui->imlYellowButton->callback( staticScreenCB, this );
	ui->imlCyanButton->callback( staticScreenCB, this );
	ui->imlMagentaButton->callback( staticScreenCB, this );
	ui->imlDarkBlueButton->callback( staticScreenCB, this ); 
	ui->imlBlackButton->callback( staticScreenCB, this ); 
	ui->imlGreyButton->callback( staticScreenCB, this );

	ui->imlOnButton->callback( staticScreenCB, this );
	ui->imlOffButton->callback( staticScreenCB, this );
	ui->imlXsecIDCounter->callback( staticScreenCB, this );
	ui->modImlButton->callback( staticScreenCB, this );
	ui->modOmlButton->callback( staticScreenCB, this );

	//==== Profile & XSec Interp Stuff ====//
	ui->numInterp1Counter->callback( staticScreenCB, this );
	ui->numInterp2Counter->callback( staticScreenCB, this );
	ui->decNumAllButton->callback( staticScreenCB, this );
	ui->incNumAllButton->callback( staticScreenCB, this );

	
	xsHeightButton = new ParmButton( this, ui->heightButton );
	xsWidthButton = new ParmButton( this, ui->widthButton );

	xsZOffsetButton = new ParmButton( this, ui->zOffsetButton );
	xsWidthOffsetButton = new ParmButton( this, ui->widthOffsetButton);
	xsCornerRadButton = new ParmButton( this, ui->cornerRadButton );
	xsTopTanAngButton = new ParmButton( this, ui->topTanButton );
	xsBotTanAngButton = new ParmButton( this, ui->botTanButton );
	xsLocButton = new ParmButton( this, ui->locationButton );

	lengthButton = new ParmButton( this, ui->lengthButton );
	camberButton = new ParmButton( this, ui->camberButton );
	camberLocButton = new ParmButton( this, ui->camberLocButton );
	aftOffsetButton = new ParmButton( this, ui->aftOffsetButton );
	noseAngleButton = new ParmButton( this, ui->noseAngleButton );
	noseStrButton = new ParmButton( this, ui->noseStrButton  );
	noseRhoButton = new ParmButton( this, ui->noseRhoButton );
	aftRhoButton = new ParmButton( this, ui->aftRhoButton );


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
FuseScreen::~FuseScreen()
{
}

void FuseScreen::show(Geom* geomPtr)
{
	char str[256];

	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);


	currGeom = (Fuse_geom*)geomPtr;
	curr_fxsec = currGeom->get_curr_xsec();

	FuseUI* ui = fuseUI;

	setTitle( currGeom->getName().get_char_star() );

	//==== Fuse Specific UI ====//
	ui->xsTypeChoice->value( curr_fxsec->get_type() );
	ui->pointSpacingChoice->value( curr_fxsec->get_pnt_space_type() );

	xsHeightSlider->set_parm_ptr( curr_fxsec->get_height() );
	xsHeightInput->set_parm_ptr( curr_fxsec->get_height() );

	xsWidthSlider->set_parm_ptr( curr_fxsec->get_width() );
	xsWidthInput->set_parm_ptr( curr_fxsec->get_width() );

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

	camberSlider->set_parm_ptr( currGeom->get_camber() );
	camberInput->set_parm_ptr(  currGeom->get_camber() );

	camberLocSlider->set_parm_ptr( currGeom->get_camber_loc() );
	camberLocInput->set_parm_ptr(  currGeom->get_camber_loc() );

	aftOffsetSlider->set_parm_ptr( currGeom->get_aft_offset()  );
	aftOffsetInput->set_parm_ptr(  currGeom->get_aft_offset()  );

	noseAngleSlider->set_parm_ptr( currGeom->get_nose_angle()  );
	noseAngleInput->set_parm_ptr(  currGeom->get_nose_angle() );

	noseStrSlider->set_parm_ptr( currGeom->get_nose_str()  );
	noseStrInput->set_parm_ptr(  currGeom->get_nose_str() );
	
	noseRhoSlider->set_parm_ptr( currGeom->get_nose_rho()  );
	noseRhoInput->set_parm_ptr(  currGeom->get_nose_rho() );
	aftRhoSlider->set_parm_ptr( currGeom->get_aft_rho()  );
	aftRhoInput->set_parm_ptr(  currGeom->get_aft_rho() );

	//==== Profile Stuff ====//
	profileTanAngSlider->set_parm_ptr( curr_fxsec->getProfileTanAng() );
	profileTanAngInput->set_parm_ptr( curr_fxsec->getProfileTanAng() );

	profileTanStr1Slider->set_parm_ptr( curr_fxsec->getProfileTanStr1() );
	profileTanStr1Input->set_parm_ptr( curr_fxsec->getProfileTanStr1() );

	profileTanStr2Slider->set_parm_ptr( curr_fxsec->getProfileTanStr2() );
	profileTanStr2Input->set_parm_ptr( curr_fxsec->getProfileTanStr2() );

	fuseUI->numInterp1Counter->value( curr_fxsec->getNumSectInterp1() );
	fuseUI->numInterp2Counter->value( curr_fxsec->getNumSectInterp2() );

	ui->noseRhoOnButton->value( currGeom->getNoseSuperFlag() );
	ui->aftRhoOnButton->value( currGeom->getAftSuperFlag() );

	//==== IML Stuff ====//
	topThickSlider->set_parm_ptr( curr_fxsec->get_top_thick() );
	topThickInput->set_parm_ptr( curr_fxsec->get_top_thick() );
	botThickSlider->set_parm_ptr( curr_fxsec->get_bot_thick() );
	botThickInput->set_parm_ptr( curr_fxsec->get_bot_thick() );
	sideThickSlider->set_parm_ptr( curr_fxsec->get_side_thick() );
	sideThickInput->set_parm_ptr( curr_fxsec->get_side_thick() );

	sprintf( str, "%6.3f", curr_fxsec->get_act_top_thick() );
	fuseUI->topThickOutput->value(str);
	sprintf( str, "%6.3f", curr_fxsec->get_act_bot_thick() );
	fuseUI->botThickOutput->value(str);
	sprintf( str, "%6.3f", curr_fxsec->get_act_side_thick() );
	fuseUI->sideThickOutput->value(str);

	if ( curr_fxsec->get_ml_type() == IML )
	{
		fuseUI->modImlButton->value(1);
		fuseUI->modOmlButton->value(0);
	}
	else
	{
		fuseUI->modImlButton->value(0);
		fuseUI->modOmlButton->value(1);
	}

	if ( curr_fxsec->get_iml_flag() )
	{
		fuseUI->imlOnButton->value(1);	
		fuseUI->imlOffButton->value(0);	
	}
	else
	{
		fuseUI->imlOnButton->value(0);	
		fuseUI->imlOffButton->value(1);	
	}

	ui->imlMaterialChoice->value( currGeom->getImlMaterialID() );

	vec3d c = currGeom->getImlColor();
	ui->imlPrimColorButton->color( fl_rgb_color((int)c.x(), (int)c.y(), (int)c.z()) );
	ui->imlRedSlider->value( c.x() );
	ui->imlGreenSlider->value( c.y() );
	ui->imlBlueSlider->value( c.z() );

	ui->imlXsecIDCounter->bounds(0, currGeom->get_num_xsecs()-1);
	ui->imlXsecIDCounter->value( currGeom->get_curr_xsec_num() );

	ui->tanXsecIDCounter->bounds(0, currGeom->get_num_xsecs()-1);
	ui->tanXsecIDCounter->value( currGeom->get_curr_xsec_num() );

//curr_fxsec->setAllTanStrFlag( 1 );
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
	xsZOffsetButton->set_parm_ptr( curr_fxsec->get_z_offset() );
	xsWidthOffsetButton->set_parm_ptr( curr_fxsec->get_max_width_loc() );
	xsCornerRadButton->set_parm_ptr( curr_fxsec->get_corner_rad() );
	xsTopTanAngButton->set_parm_ptr( curr_fxsec->get_top_tan_angle() );
	xsBotTanAngButton->set_parm_ptr( curr_fxsec->get_bot_tan_angle() );
	xsLocButton->set_parm_ptr( curr_fxsec->get_location() );

	lengthButton->set_parm_ptr( currGeom->get_length() );
	camberButton->set_parm_ptr( currGeom->get_camber() );
	camberLocButton->set_parm_ptr( currGeom->get_camber_loc() );
	aftOffsetButton->set_parm_ptr( currGeom->get_aft_offset() );
	noseAngleButton->set_parm_ptr( currGeom->get_nose_angle() );
	noseStrButton->set_parm_ptr( currGeom->get_nose_str() );
	noseRhoButton->set_parm_ptr( currGeom->get_nose_rho() );
	aftRhoButton->set_parm_ptr( currGeom->get_aft_rho() );


	ui->UIWindow->show();

	if ( curr_fxsec->get_type() == EDIT_CRV )
	{
		screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
		screenMgrPtr->getCurveEditScreen()->show();		
	}
	else
	{
		screenMgrPtr->getCurveEditScreen()->hide();	
	}

}

void FuseScreen::hide()
{

	fuseUI->UIWindow->hide();
	screenMgrPtr->getCurveEditScreen()->hide();	

}

void FuseScreen::position( int x, int y )
{
	fuseUI->UIWindow->position( x, y );
}

void FuseScreen::setTitle( const char* name )
{
	title = "FUSE : ";
	title.concatenate( name );

	fuseUI->TitleBox->label( title );
}

void FuseScreen::parm_changed( Parm* parm )
{
	char str[256];
	if ( parm )
	{
		if ( parm->get_update_grp() == UPD_FUSE_XSEC )
		{
			glWin->redraw();
		}
		if ( parm == curr_fxsec->get_top_thick() || 
			 parm == curr_fxsec->get_bot_thick() ||
			 parm == curr_fxsec->get_side_thick() )
		{
			sprintf( str, "%6.3f", curr_fxsec->get_act_top_thick() );
			fuseUI->topThickOutput->value(str);
			sprintf( str, "%6.3f", curr_fxsec->get_act_bot_thick() );
			fuseUI->botThickOutput->value(str);
			sprintf( str, "%6.3f", curr_fxsec->get_act_side_thick() );
			fuseUI->sideThickOutput->value(str);
		}

	}
}


//==== Close Callbacks =====//
void FuseScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	fuseUI->UIWindow->hide();
	screenMgrPtr->getCurveEditScreen()->hide();	
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void FuseScreen::screenCB( Fl_Widget* w)
{
	CompScreen::screenCB( w );

	if ( !currGeom )
		return;

	FuseUI* ui = fuseUI;

	if ( w == ui->xsTypeChoice  )
	{
		int val = ui->xsTypeChoice->value();
		s_type(ScriptMgr::GUI,  val);
	}
	else if ( w == ui->editButton )
	{
		if ( curr_fxsec->get_type() == EDIT_CRV )
		{
			screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
			screenMgrPtr->getCurveEditScreen()->show();	
		}
		else if ( curr_fxsec->get_type() == FROM_FILE )
		{
			curr_fxsec->set_type( EDIT_CRV );			// Hack to bring up file browser
			s_type(ScriptMgr::GUI,  FROM_FILE );
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
		/*
		curr_fxsec = currGeom->add_xsec(curr_fxsec);
		show(currGeom);
		*/
	}
	else if ( w == ui->cutButton )
	{
		s_xsec_cut(ScriptMgr::GUI);
		/*
		curr_fxsec = currGeom->delete_xsec( curr_fxsec );
		show(currGeom);
		*/
	}
	else if ( w == ui->copyButton )
	{
		s_xsec_copy(ScriptMgr::GUI);
		/*
		currGeom->copy_xsec(curr_fxsec);
		*/
	}
	else if ( w == ui->pasteButton )
	{
		s_xsec_paste(ScriptMgr::GUI);
		/*
		currGeom->paste_xsec(curr_fxsec);
		show(currGeom);
		*/
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
	else if ( w == ui->imlRedSlider || w == ui->imlGreenSlider	|| w == ui->imlBlueSlider )				
	{
		int r = (int)ui->imlRedSlider->value();
		int g = (int)ui->imlGreenSlider->value();
		int b = (int)ui->imlBlueSlider->value();

		currGeom->setImlColor( r, g, b );
		ui->imlPrimColorButton->color( fl_rgb_color(r, g, b) );
		ui->imlPrimColorButton->redraw();
	}
	else if ( w == ui->imlRedButton )
		setImlColor( 255, 0, 0 );
	else if ( w == ui->imlGreenButton )
		setImlColor( 0, 255, 0 );
	else if ( w == ui->imlBlueButton )
		setImlColor( 0, 0, 255 );
	else if ( w == ui->imlYellowButton )
		setImlColor( 255, 255, 0 );
	else if ( w == ui->imlCyanButton )
		setImlColor( 0, 255, 255 );
	else if ( w == ui->imlMagentaButton )
		setImlColor( 255, 0, 255 );
	else if ( w == ui->imlDarkBlueButton )
		setImlColor( 0, 0, 123 );
	else if ( w == ui->imlBlackButton )
		setImlColor( 0, 0, 0 );
	else if ( w == ui->imlGreyButton )
		setImlColor( 123, 123, 123 );

	else if ( w == ui->imlMaterialChoice )
	{
		currGeom->setImlMaterialID( ui->imlMaterialChoice->value() );
	}
	else if ( w == ui->imlOnButton )
	{
		currGeom->set_iml_flag( 1 );
		ui->imlOffButton->value(0);
	}
	else if ( w == ui->imlOffButton )
	{
		currGeom->set_iml_flag( 0 );
		ui->imlOnButton->value(0);
	}
	else if ( w == ui->imlXsecIDCounter )
	{
		int id = (int)ui->imlXsecIDCounter->value();
		currGeom->set_curr_xsec_num(id);
		curr_fxsec = currGeom->get_curr_xsec();
		currGeom->generate();
		show(currGeom);
	}
	else if ( w == ui->modImlButton )
	{
		curr_fxsec->set_ml_type( IML );
		ui->modOmlButton->value(0);
		show( currGeom );				// Change parm to gui associations (IML/OML)
	}
	else if ( w == ui->modOmlButton )
	{
		curr_fxsec->set_ml_type( OML );
		ui->modImlButton->value(0);
		show( currGeom );				// Change parm to gui associations (IML/OML)
	}
	else if ( w == ui->numInterp1Counter )
	{
		int val = (int)ui->numInterp1Counter->value();
		s_xsec_num_interp1(ScriptMgr::GUI, val );

//		curr_fxsec->setNumSectInterp1( ui->numInterp1Counter->value() );
//		currGeom->updateNumInter();
//		currGeom->generate();
//		show(currGeom);
	}
	else if ( w == ui->numInterp2Counter )
	{
		int val = (int)ui->numInterp2Counter->value();
		s_xsec_num_interp2(ScriptMgr::GUI, val );

//		curr_fxsec->setNumSectInterp2( ui->numInterp2Counter->value() );
//		currGeom->updateNumInter();
//		currGeom->generate();
//		show(currGeom);
	}
	else if ( w == ui->decNumAllButton )
	{
		s_xsec_dec_num_all(ScriptMgr::GUI);

//		currGeom->incNumInterpAll( -1 );
//		currGeom->generate();
//		show(currGeom);
	}
	else if ( w == ui->incNumAllButton )
	{
		s_xsec_inc_num_all(ScriptMgr::GUI);

//		currGeom->incNumInterpAll( +1 );
//		currGeom->generate();
//		show(currGeom);
	}
	else if ( w == ui->noseRhoOnButton )
	{
		int val = ui->noseRhoOnButton->value();
		s_xsec_nose_rho_on(ScriptMgr::GUI, val);

//		currGeom->setNoseSuperFlag( ui->noseRhoOnButton->value() );
//		currGeom->generate();
//		show(currGeom);
	}
	else if ( w == ui->aftRhoOnButton )
	{
		int val = ui->aftRhoOnButton->value();
		s_xsec_aft_rho_on(ScriptMgr::GUI, val);

//		currGeom->setAftSuperFlag( ui->aftRhoOnButton->value() );
//		currGeom->generate();
//		show(currGeom);
	}

}

void FuseScreen::setImlColor( int r, int g, int b )
{
	FuseUI* ui = fuseUI;

	ui->imlRedSlider->value(r);
	ui->imlGreenSlider->value(g);
	ui->imlBlueSlider->value(b);
	currGeom->setImlColor( r, g, b );

	ui->imlPrimColorButton->color( fl_rgb_color(r, g, b) );
	ui->imlPrimColorButton->redraw();
}





void FuseScreen::s_set_id(int src, int id) 
{
	currGeom->set_curr_xsec_num(id);
	curr_fxsec = currGeom->get_curr_xsec();
	currGeom->generate();

	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec", "id", id);
	}
}


void FuseScreen::s_xsec_add(int src) {
	curr_fxsec = currGeom->add_xsec(curr_fxsec);
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec add");
	}
}

void FuseScreen::s_xsec_cut(int src) {
		curr_fxsec = currGeom->delete_xsec( curr_fxsec );
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec cut");
	}
}

void FuseScreen::s_xsec_copy(int src) {
	currGeom->copy_xsec(curr_fxsec);
	if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec copy");
}

void FuseScreen::s_xsec_paste(int src) {
	currGeom->paste_xsec(curr_fxsec);
	if (src != ScriptMgr::SCRIPT)
	{
		show(currGeom);
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec paste");
	}
}

void FuseScreen::s_type(int src, int val)
{
	int old_type = curr_fxsec->get_type();
	curr_fxsec->set_type( val );

	if (src == ScriptMgr::SCRIPT)
	{

		if ( curr_fxsec->get_type() == RND_BOX )
		{
			curr_fxsec->setAllTanStrFlag( 1 );
		}

		curr_fxsec->regenerate();
		currGeom->generate();
	}
	else
	{
		FuseUI* ui = fuseUI;

		if ( curr_fxsec->get_type() == GENERAL )
		{
			ui->tanStrChoice->activate();
		}
		else if ( curr_fxsec->get_type() == RND_BOX )
		{
			ui->tanStrChoice->value(0);	
			curr_fxsec->setAllTanStrFlag( 1 );
			xsTanStrSlider->set_parm_ptr( curr_fxsec->get_top_str() );
			xsTanStrInput->set_parm_ptr(  curr_fxsec->get_top_str() );
		}
		else if ( curr_fxsec->get_type() == FROM_FILE  )
		{
			if ( old_type != FROM_FILE )
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
					ui->xsTypeChoice->value(GENERAL);
					curr_fxsec->set_type( GENERAL );
					ui->tanStrChoice->activate();
				}
			}
		}
		else if ( curr_fxsec->get_type() == EDIT_CRV  )
		{
			screenMgrPtr->getCurveEditScreen()->setEditCurvePtr( curr_fxsec->getEditCrv() );
			screenMgrPtr->getCurveEditScreen()->show();			
		}
		else
		{
			ui->tanStrChoice->deactivate();
		}

		if ( curr_fxsec->get_type() != EDIT_CRV )
			screenMgrPtr->getCurveEditScreen()->hide();			


		curr_fxsec->regenerate();
		currGeom->generate();
		glWin->redraw();

		if (src == ScriptMgr::GUI)
		{
			switch (val)
			{
				case XSEC_POINT:
					scriptMgr->addLine("fuse_xsec type point");
					break;
				case CIRCLE:
					scriptMgr->addLine("fuse_xsec type circle");
					break;
				case ELLIPSE:
					scriptMgr->addLine("fuse_xsec type ellipse");
					break;
				case RND_BOX:
					scriptMgr->addLine("fuse_xsec type roundbox");
					break;
				case GENERAL:
					scriptMgr->addLine("fuse_xsec type general");
					break;
				case FROM_FILE:
					scriptMgr->addLine("fuse_xsec type file");
					break;
				case EDIT_CRV:
					scriptMgr->addLine("fuse_xsec type edit");
					break;
			}
		}
	}
}

void FuseScreen::s_xsec_num_interp1(int src, int val)
{
	curr_fxsec->setNumSectInterp1( val );
	currGeom->updateNumInter();
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec", "xsec_num_interp1", val);
	}
}

void FuseScreen::s_xsec_num_interp2(int src, int val)
{
	curr_fxsec->setNumSectInterp2( val );
	currGeom->updateNumInter();
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_xsec", "xsec_num_interp2", val);
	}
}

void FuseScreen::s_xsec_dec_num_all(int src)
{
	currGeom->incNumInterpAll( -1 );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_shape", "xsec_dec_num_all");
	}
}

void FuseScreen::s_xsec_inc_num_all(int src)
{
	currGeom->incNumInterpAll( 1 );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_shape", "xsec_inc_num_all");
	}
}

void FuseScreen::s_xsec_nose_rho_on(int src, int val)
{
	currGeom->setNoseSuperFlag( val );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_shape", "xsec_nose_rho_on", val);
	}
}

void FuseScreen::s_xsec_aft_rho_on(int src, int val)
{
	currGeom->setAftSuperFlag( val );
	currGeom->generate();
	show(currGeom);

	if (src != ScriptMgr::SCRIPT)
	{
		if (src == ScriptMgr::GUI) scriptMgr->addLine("fuse_shape", "xsec_aft_rho_on", val);
	}
}

