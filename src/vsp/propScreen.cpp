//****************************************************************************
//    
//   Prop Geom Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "propScreen.h"
#include "propGeom.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//==== Constructor =====//
PropScreen::PropScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	PropUI* ui = propUI = new PropUI();

	propUI->UIWindow->position( 760, 50 );
	propUI->UIWindow->callback( staticCloseCB, this );
	
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

	//===== Prop Stuff ====//
	ui->numBladesCounter->callback( staticScreenCB, this );
	ui->stationCounter->callback( staticScreenCB, this );
	ui->station2Counter->callback( staticScreenCB, this );
	ui->numUCounter->callback( staticScreenCB, this );
	ui->numWCounter->callback( staticScreenCB, this );
	ui->smoothButton->callback( staticScreenCB, this );
	ui->addStationButton->callback( staticScreenCB, this );
	ui->delStationButton->callback( staticScreenCB, this );

	ui->numBladesCounter->lstep(10.0);

	diameterSlider = new Slider_adj_range(this, ui->diameterSlider, ui->diameterButtonL, ui->diameterButtonR, 10.0 );
	diameterInput  = new Input( this, ui->diameterInput );	
	coneAngleSlider = new Slider_adj_range(this, ui->coneAngSlider, ui->coneAngButtonL, ui->coneAngButtonR, 10.0 );
	coneAngleInput  = new Input( this, ui->coneAngInput );
	pitchSlider = new Slider_adj_range(this, ui->pitchSlider, ui->pitchButtonL, ui->pitchButtonR, 30.0 );
	pitchInput  = new Input( this, ui->pitchInput );

	chordSlider = new Slider_adj_range(this, ui->chordSlider, ui->chordButtonL, ui->chordButtonR, 10.0 );
	chordInput  = new Input( this, ui->chordInput );	
	twistSlider = new Slider_adj_range(this, ui->twistSlider, ui->twistButtonL, ui->twistButtonR, 30.0 );
	twistInput  = new Input( this, ui->twistInput );	
	locSlider = new Slider_adj_range(this, ui->locSlider, ui->locButtonL, ui->locButtonR, 1.0 );
	locInput  = new Input( this, ui->locInput );	
	offSlider = new Slider_adj_range(this, ui->offsetSlider, ui->offsetButtonL, ui->offsetButtonR, 0.1 );
	offInput  = new Input( this, ui->offsetInput );	

	diameterButton = new ParmButton( this, ui->diameterButton  );
	coneAngleButton = new ParmButton( this, ui->coneAngButton  );
	pitchButton = new ParmButton( this, ui->pitchButton );

	chordButton = new ParmButton( this, ui->chordButton );
	twistButton = new ParmButton( this, ui->twistButton );
	locButton = new ParmButton( this, ui->locButton );
	offButton = new ParmButton( this, ui->offsetButton );

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
PropScreen::~PropScreen()
{
}

void PropScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (PropGeom*)geomPtr;
	PropUI* ui = propUI;

	setTitle( currGeom->getName().get_char_star() );

	diameterSlider->set_parm_ptr( currGeom->get_diameter() );
	diameterInput->set_parm_ptr( currGeom->get_diameter() );
	coneAngleSlider->set_parm_ptr( currGeom->get_cone_angle() );
	coneAngleInput->set_parm_ptr( currGeom->get_cone_angle() );
	pitchSlider->set_parm_ptr( currGeom->get_pitch() );
	pitchInput->set_parm_ptr( currGeom->get_pitch());

	chordSlider->set_parm_ptr( currGeom->get_chord() );
	chordInput->set_parm_ptr(  currGeom->get_chord() );
	twistSlider->set_parm_ptr( currGeom->get_twist() );
	twistInput->set_parm_ptr(  currGeom->get_twist() );
	locSlider->set_parm_ptr( currGeom->get_loc() );
	locInput->set_parm_ptr(  currGeom->get_loc() );
	offSlider->set_parm_ptr( currGeom->get_offset() );
	offInput->set_parm_ptr(  currGeom->get_offset() );

	propUI->numBladesCounter->value( currGeom->getNumBlades() );
	propUI->stationCounter->value( currGeom->getCurrSectID() );
	propUI->station2Counter->value( currGeom->getCurrSectID() );

	propUI->numUCounter->value( currGeom->getNumU() );
	propUI->numWCounter->value( currGeom->getNumW() );
	propUI->smoothButton->value( currGeom->getSmoothFlag() );

	char str[256];
	sprintf( str, "   %d", currGeom->getNumStations() );
	propUI->numStationsInput->value( str );

	//==== Airfoil Stuff ====//
	Af* afp = currGeom->get_af_ptr();
	propUI->airfoilTypeChoice->value( afp->get_type()-1 );

	propUI->airfoilInvertButton->value( afp->get_inverted_flag() );

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


	propUI->airfoilNameInput->value( afp->get_name() );
	propUI->airfoilNameInput2->value( afp->get_name() );

	if ( afp->get_type() == NACA_6_SERIES )
		propUI->sixSeriesChoice->activate();
	else
		propUI->sixSeriesChoice->deactivate();

	leRadiusInput->set_parm_ptr( afp->get_leading_edge_radius() );

	afCamberButton->set_parm_ptr( afp->get_camber() );
	afCamberLocButton->set_parm_ptr( afp->get_camber_loc() );
	afThickButton->set_parm_ptr( afp->get_thickness() );
	afThickLocButton->set_parm_ptr( afp->get_thickness_loc() );
	afIdealClButton->set_parm_ptr( afp->get_ideal_cl() );
	afAButton->set_parm_ptr( afp->get_a() );

	diameterButton->set_parm_ptr( currGeom->get_diameter() );
	coneAngleButton->set_parm_ptr( currGeom->get_cone_angle() );
	pitchButton->set_parm_ptr( currGeom->get_pitch() );

	Section* currSect = currGeom->get_curr_section();

	chordButton->set_parm_ptr( &currSect->chord );
	twistButton->set_parm_ptr( &currSect->twist );
	locButton->set_parm_ptr( &currSect->x_off );
	offButton->set_parm_ptr( &currSect->y_off );

	glWin->setDrawBase( afp );
	glWin->redraw();

	ui->UIWindow->show();

}

void PropScreen::parm_changed( Parm* parm )
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

void PropScreen::updateName()
{
	Af* afp = currGeom->get_af_ptr();
	propUI->airfoilNameInput->value( afp->get_name() );

}
void PropScreen::hide()
{
	propUI->UIWindow->hide();
}

void PropScreen::position( int x, int y )
{
	propUI->UIWindow->position( x, y );
}

void PropScreen::setTitle( const char* name )
{
	title = "PROP : ";
	title.concatenate( name );

	propUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void PropScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	propUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void PropScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

	if ( w == propUI->numBladesCounter )
	{
		currGeom->setNumBlades((int)propUI->numBladesCounter->value());
		scriptMgr->addLine("prop_shape", "numblades", currGeom->getNumBlades());
	}
	else if ( w == propUI->stationCounter )
	{
		currGeom->setCurrSectID( (int)propUI->stationCounter->value() );
		show( currGeom );
		scriptMgr->addLine("prop_station", "select", currGeom->getCurrSectID());
	}
	else if ( w == propUI->station2Counter )
	{
		currGeom->setCurrSectID( (int)propUI->station2Counter->value() );
		show( currGeom );
		scriptMgr->addLine("prop_station", "select", currGeom->getCurrSectID());
	}
	else if ( w == propUI->numUCounter )
	{
		currGeom->setNumU( (int)propUI->numUCounter->value() );
		scriptMgr->addLine("prop_shape", "numu", currGeom->getNumU());
	}
	else if ( w == propUI->numWCounter )
	{
		currGeom->setNumW( (int)propUI->numWCounter->value() );
		scriptMgr->addLine("prop_shape", "numw", currGeom->getNumW());
	}
	else if ( w == propUI->smoothButton )
	{
		currGeom->setSmoothFlag( propUI->smoothButton->value() );
		scriptMgr->addLine("prop_shape", "smooth", currGeom->getSmoothFlag());
	}
	else if ( w == propUI->addStationButton )
	{
		currGeom->addStation();
		show( currGeom );
		scriptMgr->addLine("prop_station", "add");
	}
	else if ( w == propUI->delStationButton )
	{
		currGeom->delStation();
		show( currGeom );
		scriptMgr->addLine("prop_station", "del");
	}

	//==== Airfoil GUI ====//
	else if ( w == propUI->airfoilTypeChoice )
	{
		int aftype = propUI->airfoilTypeChoice->value()+1;

		if ( aftype == AIRFOIL_FILE )
		{
			
//			char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
			char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
			if ( newfile != NULL )
			{
				currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
				scriptMgr->addLine("prop_foil", "read", newfile);
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
			scriptMgr->addLine("prop_foil", "type", "naca4");
			break;
		case BICONVEX:
			scriptMgr->addLine("prop_foil", "type", "biconvex");
			break;
		case WEDGE:
			scriptMgr->addLine("prop_foil", "type", "wedge");
			break;
		case NACA_6_SERIES:
			scriptMgr->addLine("prop_foil", "type", "naca6");
			break;

		}

	}
	else if ( w ==  propUI->readFileButton )
	{

//		char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
		char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
		if ( newfile != NULL )
		{
			currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
			scriptMgr->addLine("prop_foil", "read", newfile);
		}
		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  propUI->sixSeriesChoice  )
	{
		int choice = propUI->sixSeriesChoice->value();

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
			scriptMgr->addLine("prop_foil", "6series", "63");
			break;
		case 1:
			scriptMgr->addLine("prop_foil", "6series", "64");
			break;
		case 2:
			scriptMgr->addLine("prop_foil", "6series", "65");
			break;
		case 3:
			scriptMgr->addLine("prop_foil", "6series", "66");
			break;
		case 4:
			scriptMgr->addLine("prop_foil", "6series", "67");
			break;
		case 5:
			scriptMgr->addLine("prop_foil", "6series", "63a");
			break;
		case 6:
			scriptMgr->addLine("prop_foil", "6series", "64a");
			break;
		case 7:
			scriptMgr->addLine("prop_foil", "6series", "65a");
			break;
		}
	}
	else if ( w ==  propUI->airfoilInvertButton   )
	{
		int flag = propUI->airfoilInvertButton->value();

		currGeom->get_af_ptr()->set_inverted_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

		scriptMgr->addLine("prop_foil", "inverty", flag);
	}

}


