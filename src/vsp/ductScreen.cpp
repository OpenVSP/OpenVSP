//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//****************************************************************************
//    
//   Duct Geom Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "ductScreen.h"
#include "ductGeom.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//==== Constructor =====//
DuctScreen::DuctScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	DuctUI* ui = ductUI = new DuctUI();

	ductUI->UIWindow->position( 760, 30 );
	ductUI->UIWindow->callback( staticCloseCB, this );
	
	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	//==== Airfoil Stuff ====//
	ui->airfoilNameInput->readonly(1);
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

	//===== Duct Stuff ====//
	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 10.0 );
	lengthInput  = new Input( this, ui->lengthInput );

	inletOutletSlider = new Slider_adj_range(this, ui->inletOutletSlider, ui->inletOutletButtonL, ui->inletOutletButtonR, 0.5 );
	inletOutletInput  = new Input( this, ui->inletOutletInput );

	inletDiaSlider = new Slider_adj_range(this, ui->inletDiaSlider, ui->inletDiaButtonL, ui->inletDiaButtonR, 10.0 );
	inletDiaInput  = new Input( this, ui->inletDiaInput );
	inletAreaInput  = new Input( this, ui->inletAreaInput );

	outletDiaInput  = new Input( this, ui->outletDiaInput );
	outletAreaInput  = new Input( this, ui->outletAreaInput );

	chordInput  = new Input( this, ui->chordInput );

	
	lengthButton = new ParmButton( this, ui->lengthButton );
	inletOutletButton = new ParmButton( this, ui->inletOutletButton  );
	inletDiaButton = new ParmButton( this, ui->inletDiaButton  );

	afCamberButton = new ParmButton( this, ui->camberButton );
	afCamberLocButton = new ParmButton( this, ui->camberLocButton );
	afThickButton = new ParmButton( this, ui->thickButton );
	afThickLocButton = new ParmButton( this, ui->thickLocButton );
	afIdealClButton = new ParmButton( this, ui->idealClButton );
	afAButton = new ParmButton( this, ui->aButton );



	//==== Initail General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
DuctScreen::~DuctScreen()
{
	//delete afCamberSlider;
	//delete afCamberInput;
	//delete afCamberLocSlider;
	//delete afCamberLocInput;
	//delete afThickSlider;
	//delete afThickInput;
	//delete afThickLocSlider;
	//delete afThickLocInput;
	//delete afIdealClSlider;
	//delete afIdealClInput;
	//delete afASlider;
	//delete afAInput;
	//delete leRadiusInput;
	//delete lengthSlider;
	//delete lengthInput;
	//delete inletOutletSlider;
	//delete inletOutletInput;
	//delete inletDiaSlider;
	//delete inletDiaInput;
	//delete inletAreaInput;
	//delete outletDiaInput;
	//delete outletAreaInput;
	//delete chordInput;
	//delete lengthButton;
	//delete inletOutletButton;
	//delete inletDiaButton;
	//delete afCamberButton;
	//delete afCamberLocButton;
	//delete afThickButton;
	//delete afThickLocButton;
	//delete afIdealClButton;
	//delete afAButton;
}

void DuctScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (DuctGeom*)geomPtr;
	DuctUI* ui = ductUI;

	setTitle( currGeom->getName().get_char_star() );

	lengthSlider->set_parm_ptr( currGeom->get_length() );
	lengthInput->set_parm_ptr( currGeom->get_length() );
	inletOutletSlider->set_parm_ptr( currGeom->get_inlet_outlet() );
	inletOutletInput->set_parm_ptr( currGeom->get_inlet_outlet() );

	inletDiaSlider->set_parm_ptr( currGeom->get_inlet_dia() );
	inletDiaInput->set_parm_ptr( currGeom->get_inlet_dia() );
	inletAreaInput->set_parm_ptr( currGeom->get_inlet_area() );

	outletDiaInput->set_parm_ptr( currGeom->get_outlet_dia() );
	outletAreaInput->set_parm_ptr( currGeom->get_outlet_area() );

	chordInput->set_parm_ptr( currGeom->get_chord() );

	//==== Airfoil Stuff ====//
	Af* afp = currGeom->get_af_ptr();
	ductUI->airfoilTypeChoice->value( afp->get_type()-1 );

	ductUI->airfoilInvertButton->value( afp->get_inverted_flag() );

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

	ductUI->airfoilNameInput->value( afp->get_name() );

	if ( afp->get_type() == NACA_6_SERIES )
		ductUI->sixSeriesChoice->activate();
	else
		ductUI->sixSeriesChoice->deactivate();

	leRadiusInput->set_parm_ptr( afp->get_leading_edge_radius() );

	lengthButton->set_parm_ptr( currGeom->get_length() );
	inletOutletButton->set_parm_ptr( currGeom->get_inlet_outlet() );
	inletDiaButton->set_parm_ptr( currGeom->get_inlet_dia() );

	afCamberButton->set_parm_ptr( afp->get_camber() );
	afCamberLocButton->set_parm_ptr( afp->get_camber_loc() );
	afThickButton->set_parm_ptr( afp->get_thickness() );
	afThickLocButton->set_parm_ptr( afp->get_thickness_loc() );
	afIdealClButton->set_parm_ptr( afp->get_ideal_cl() );
	afAButton->set_parm_ptr( afp->get_a() );


	glWin->setDrawBase( afp );
	glWin->redraw();



	ui->UIWindow->show();

}

void DuctScreen::parm_changed( Parm* parm )
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

void DuctScreen::updateName()
{
	Af* afp = currGeom->get_af_ptr();
	ductUI->airfoilNameInput->value( afp->get_name() );

}
void DuctScreen::hide()
{
	ductUI->UIWindow->hide();
}

void DuctScreen::position( int x, int y )
{
	ductUI->UIWindow->position( x, y );
}

void DuctScreen::setTitle( const char* name )
{
	title = "DUCT : ";
	title.concatenate( name );

	ductUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void DuctScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	ductUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void DuctScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

	if ( w == ductUI->airfoilTypeChoice )
	{
		int aftype = ductUI->airfoilTypeChoice->value()+1;

		if ( aftype == AIRFOIL_FILE )
		{

//			char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
			char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
			if ( newfile != NULL )
			{
				currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
				scriptMgr->addLine("duct_foil", "read", newfile);
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

		switch ( aftype )
		{
		case NACA_4_SERIES:
			scriptMgr->addLine("duct_foil", "type", "naca4");
			break;
		case BICONVEX:
			scriptMgr->addLine("duct_foil", "type", "biconvex");
			break;
		case WEDGE:
			scriptMgr->addLine("duct_foil", "type", "wedge");
			break;
		case NACA_6_SERIES:
			scriptMgr->addLine("duct_foil", "type", "naca6");
			break;
		}

	}
	else if ( w ==  ductUI->readFileButton )
	{

//		char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
		char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af" );
		if ( newfile != NULL )
		{
			currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
			scriptMgr->addLine("duct_foil", "read", newfile);
		}
		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  ductUI->sixSeriesChoice  )
	{
		int choice = ductUI->sixSeriesChoice->value();

		if ( choice <= 4 )
			currGeom->get_af_ptr()->set_sixser( choice + 63 );
		else
			currGeom->get_af_ptr()->set_sixser( -(choice + 58) );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

		switch (choice)
		{
		case 0:
			scriptMgr->addLine("duct_foil", "6series", "63");
			break;
		case 1:
			scriptMgr->addLine("duct_foil", "6series", "64");
			break;
		case 2:
			scriptMgr->addLine("duct_foil", "6series", "65");
			break;
		case 3:
			scriptMgr->addLine("duct_foil", "6series", "66");
			break;
		case 4:
			scriptMgr->addLine("duct_foil", "6series", "67");
			break;
		case 5:
			scriptMgr->addLine("duct_foil", "6series", "63a");
			break;
		case 6:
			scriptMgr->addLine("duct_foil", "6series", "64a");
			break;
		case 7:
			scriptMgr->addLine("duct_foil", "6series", "65a");
			break;
		}
	}
	else if ( w ==  ductUI->airfoilInvertButton   )
	{
		int flag = ductUI->airfoilInvertButton->value();

		currGeom->get_af_ptr()->set_inverted_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

		scriptMgr->addLine("duct_foil", "inverty", flag);
	}
}


