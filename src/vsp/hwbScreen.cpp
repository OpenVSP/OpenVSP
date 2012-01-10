//******************************************************************************
//
//   Hybrid Wing Body Screen Class
//
//
//   Michael Link - 6/11/08
//   AVID LLC
//
//
//******************************************************************************

#include "hwbScreen.h"
#include "af.h"
#include "scriptMgr.h"

HwbScreen::HwbScreen(ScreenMgr* mgrPtr) : CompScreen( mgrPtr )
{
	HybridWingBodyUI* ui = hwbUI = new HybridWingBodyUI();

	hwbUI->UIWindow->position( 760, 30 );
	hwbUI->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	//==== Wing Stuff ====//
	totalSpanSlider = new Slider_adj_range(this, ui->totalSpanSlider, ui->totalSpanButtonL, ui->totalSpanButtonR, 20.0 );
	totalSpanInput  = new Input( this, ui->totalSpanInput );
	totalProjSpanSlider = new Slider_adj_range(this, ui->totalProjSpanSlider, ui->totalProjSpanButtonL, ui->totalProjSpanButtonR, 20.0 );
	totalProjSpanInput  = new Input( this, ui->totalProjSpanInput );
	totalChordSlider = new Slider_adj_range(this, ui->totalChordSlider, ui->totalChordButtonL, ui->totalChordButtonR, 4.0 );
	totalChordInput  = new Input( this, ui->totalChordInput );
	totalAreaSlider = new Slider_adj_range(this, ui->totalAreaSlider, ui->totalAreaButtonL, ui->totalAreaButtonR, 100.0 );
	totalAreaInput  = new Input( this, ui->totalAreaInput );

	totalARInput = new Input( this, ui->totalARInput );
	sweepOffsetSlider = new Slider_adj_range(this, ui->totalSweepOffSlider, ui->totalSweepOffButtonL, ui->totalSweepOffButtonR, 4.0 );
	sweepOffsetInput  = new Input( this, ui->totalSweepOffInput );

	ARSlider = new Slider_adj_range(this, ui->ARSlider, ui->ARButtonL, ui->ARButtonR, 1.0 );
	ARInput = new Input( this, ui->ARInput );
	TRSlider = new Slider_adj_range(this, ui->TRSlider, ui->TRButtonL, ui->TRButtonR, 1.0 );
	TRInput = new Input( this, ui->TRInput );
	areaSlider = new Slider_adj_range(this, ui->areaSlider, ui->areaButtonL, ui->areaButtonR, 100.0 );
	areaInput = new Input( this, ui->areaInput );
	spanSlider = new Slider_adj_range(this, ui->spanSlider, ui->spanButtonL, ui->spanButtonR, 10.0 );
	spanInput = new Input( this, ui->spanInput );
	TCSlider = new Slider_adj_range(this, ui->TCSlider, ui->TCButtonL, ui->TCButtonR, 10.0 );
	TCInput = new Input( this, ui->TCInput );
	RCSlider = new Slider_adj_range(this, ui->RCSlider, ui->RCButtonL, ui->RCButtonR, 10.0 );
	RCInput = new Input( this, ui->RCInput );

	hwbUI->driverChoice->callback( staticScreenCB, this );

	sectProjSpanOutput = new Input( this, ui->projSpanOutput );

	sweepSlider = new Slider_adj_range(this, ui->sweepSlider, ui->sweepButtonL, ui->sweepButtonR, 20.0 );
	sweepInput = new Input( this, ui->sweepInput );
	sweepLocSlider = new Slider_adj_range(this, ui->sweepLocSlider, ui->sweepLocButtonL, ui->sweepLocButtonR, 1.0 );
	sweepLocInput = new Input( this, ui->sweepLocInput );
	twistSlider = new Slider_adj_range(this, ui->twistSlider, ui->twistButtonL, ui->twistButtonR, 1.0 );
	twistInput = new Input( this, ui->twistInput );
	twistLocSlider = new Slider_adj_range(this, ui->twistLocSlider, ui->twistLocButtonL, ui->twistLocButtonR, 5.0 );
	twistLocInput = new Input( this, ui->twistLocInput );
	dihed1Slider = new Slider_adj_range(this, ui->dihed1Slider, ui->dihed1ButtonL, ui->dihed1ButtonR, 10.0 );
	dihed1Input = new Input( this, ui->dihed1Input );

	sweepPercentSpan1Slider = new Slider_adj_range(this, ui->sweepPercentSpan1Slider, ui->sweepPercentSpan1ButtonL, ui->sweepPercentSpan1ButtonR, 0.5 );
	sweepPercentSpan1Input = new Input( this, ui->sweepPercentSpan1Input );
	tesweepPercentSpan1Slider = new Slider_adj_range(this, ui->tesweepPercentSpan1Slider, ui->tesweepPercentSpan1ButtonL, ui->tesweepPercentSpan1ButtonR, 0.5 );
	tesweepPercentSpan1Input = new Input( this, ui->tesweepPercentSpan1Input );
	dihedralPercentSpan1Slider = new Slider_adj_range(this, ui->dihedralPercentSpan1Slider, ui->dihedralPercentSpan1ButtonL, ui->dihedralPercentSpan1ButtonR, 0.5 );
	dihedralPercentSpan1Input = new Input( this, ui->dihedralPercentSpan1Input );

	sweepDegPerSegInput = new Input( this, ui->sweepDegPerSegInput );
	sweepDegPerSegInput->set_format( "%1.0f" );

	tesweepDegPerSegInput = new Input( this, ui->tesweepDegPerSegInput );
	tesweepDegPerSegInput->set_format( "%1.0f" );

	dihedralDegPerSegInput = new Input( this, ui->dihedralDegPerSegInput );
	dihedralDegPerSegInput->set_format( "%1.0f" );

	filletedTipChordOutput = new Input( this, ui->filletedTipChordOutput );
	filletedRootChordOutput = new Input( this, ui->filletedRootChordOutput );

	//ui->rotateAirfoilButton->deactivate();
	ui->rotateAirfoilButton->callback( staticScreenCB, this );
	ui->numInterpCounter->callback( staticScreenCB, this );
	ui->incAllInterpButton->callback( staticScreenCB, this );
	ui->decAllInterpButton->callback( staticScreenCB, this );

	ui->sectionCounter->callback( staticScreenCB, this );
	ui->jointCounter->callback( staticScreenCB, this );
	ui->airfoilCounter->callback( staticScreenCB, this );

	ui->addSectButton->callback( staticScreenCB, this );
	ui->insertSectButton->callback( staticScreenCB, this );
	ui->copySectButton->callback( staticScreenCB, this ); 
	ui->pasteSectButton->callback( staticScreenCB, this ); 
	ui->delSectButton->callback( staticScreenCB, this ); 

	//==== Airfoil Stuff ====//
	ui->airfoilNameInput->readonly(1);
	ui->airfoilTypeChoice->callback( staticScreenCB, this );
	ui->readFileButton->callback( staticScreenCB, this );
	ui->sixSeriesChoice->callback( staticScreenCB, this );
	ui->airfoilInvertButton->callback( staticScreenCB, this );
	ui->slatButton->callback( staticScreenCB, this );
	ui->slatShearButton->callback( staticScreenCB, this );
	ui->flapButton->callback( staticScreenCB, this );
	ui->flapShearButton->callback( staticScreenCB, this );


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

	afSlatChordSlider = new Slider_adj_range(this, ui->slatChordSlider, ui->slatChordButtonL, ui->slatChordButtonR, 0.5 );
	afSlatChordInput = new Input( this, ui->slatChordInput );
	afSlatAngleSlider = new Slider_adj_range(this, ui->slatAngleSlider, ui->slatAngleButtonL, ui->slatAngleButtonR, 45.0 );
	afSlatAngleInput = new Input( this, ui->slatAngleInput );
	afFlapChordSlider = new Slider_adj_range(this, ui->flapChordSlider, ui->flapChordButtonL, ui->flapChordButtonR, 0.5 );
	afFlapChordInput = new Input( this, ui->flapChordInput );
	afFlapAngleSlider = new Slider_adj_range(this, ui->flapAngleSlider, ui->flapAngleButtonL, ui->flapAngleButtonR, 45.0 );
	afFlapAngleInput = new Input( this, ui->flapAngleInput );

	leRadiusInput = new Input( this, ui->leRadInput );

	totalSpanButton = new ParmButton( this, ui->totalSpanButton  );
	totalProjSpanButton = new ParmButton( this, ui->totalProjSpanButton );
	totalChordButton = new ParmButton( this, ui->totalChordButton );
	totalAreaButton = new ParmButton( this, ui->totalAreaButton );
	sweepOffsetButton = new ParmButton( this, ui->totalSweepOffButton );

	ui->MenuTabs->callback( staticScreenCB, this );

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->airfoilGlWinGroup;
	ui->airfoilGlWinGroup->begin();
	glWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->airfoilGlWinGroup->end();

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();
}

HwbScreen::~HwbScreen()
{

}

void HwbScreen::show(Geom* geomPtr)
{
	char str[256];

	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (Hwb_geom*)geomPtr;

	setTitle( currGeom->getName().get_char_star() );

	//==== Wing Stuff ====//
	totalSpanSlider->set_parm_ptr( currGeom->get_total_span() );
	totalSpanInput->set_parm_ptr( currGeom->get_total_span() );

	totalProjSpanSlider->set_parm_ptr( currGeom->get_total_proj_span() );
	totalProjSpanInput->set_parm_ptr( currGeom->get_total_proj_span() );

	totalChordSlider->set_parm_ptr( currGeom->get_avg_chord() );
	totalChordInput->set_parm_ptr( currGeom->get_avg_chord() );

	totalAreaSlider->set_parm_ptr( currGeom->get_total_area() );
	totalAreaInput->set_parm_ptr( currGeom->get_total_area() );

	totalARInput->set_parm_ptr( currGeom->get_total_aspect() );
	totalARInput->deactivate();

	sweepOffsetSlider->set_parm_ptr( currGeom->get_sweep_off() );
	sweepOffsetInput->set_parm_ptr( currGeom->get_sweep_off() );


	ARSlider->set_parm_ptr( currGeom->get_sect_aspect() );
	ARInput->set_parm_ptr( currGeom->get_sect_aspect() );
	TRSlider->set_parm_ptr( currGeom->get_sect_taper() );
	TRInput->set_parm_ptr( currGeom->get_sect_taper() );
	areaSlider->set_parm_ptr( currGeom->get_sect_area() );
	areaInput->set_parm_ptr( currGeom->get_sect_area() );
	spanSlider->set_parm_ptr( currGeom->get_sect_span() );
	spanInput->set_parm_ptr( currGeom->get_sect_span() );
	RCSlider->set_parm_ptr( currGeom->get_sect_rc() );
	RCInput->set_parm_ptr( currGeom->get_sect_rc() );
	TCSlider->set_parm_ptr( currGeom->get_sect_tc() );
	TCInput->set_parm_ptr( currGeom->get_sect_tc() );

	hwbUI->driverChoice->value( currGeom->get_driver() ); 

	sectProjSpanOutput->set_parm_ptr( currGeom->get_sect_proj_span() ); 
	sectProjSpanOutput->deactivate();

	sweepSlider->set_parm_ptr( currGeom->get_sect_sweep() );
	sweepInput->set_parm_ptr( currGeom->get_sect_sweep() );
	sweepLocSlider->set_parm_ptr( currGeom->get_sect_sweep_loc() );
	sweepLocInput->set_parm_ptr( currGeom->get_sect_sweep_loc() );
	twistSlider->set_parm_ptr( currGeom->get_sect_twist() );
	twistInput->set_parm_ptr( currGeom->get_sect_twist() );
	twistLocSlider->set_parm_ptr( currGeom->get_sect_twist_loc() );
	twistLocInput->set_parm_ptr( currGeom->get_sect_twist_loc() );
	dihed1Slider->set_parm_ptr( currGeom->get_sect_dihed1() );
	dihed1Input->set_parm_ptr( currGeom->get_sect_dihed1() );

	filletedTipChordOutput->set_parm_ptr( currGeom->get_filleted_tip_chord() );
	filletedTipChordOutput->deactivate();
	filletedRootChordOutput->set_parm_ptr( currGeom->get_filleted_root_chord() );
	filletedRootChordOutput->deactivate();

	//==== Fillet Stuff ====//	
	sweepPercentSpan1Slider->set_parm_ptr( currGeom->get_sweep_percent_edge_length() );
	sweepPercentSpan1Input->set_parm_ptr( currGeom->get_sweep_percent_edge_length() );
	sweepDegPerSegInput->set_parm_ptr( currGeom->get_sweep_degperseg() );

	tesweepPercentSpan1Slider->set_parm_ptr( currGeom->get_tesweep_percent_edge_length() );
	tesweepPercentSpan1Input->set_parm_ptr( currGeom->get_tesweep_percent_edge_length() );
	tesweepDegPerSegInput->set_parm_ptr( currGeom->get_tesweep_degperseg() );

	dihedralPercentSpan1Slider->set_parm_ptr( currGeom->get_dihedral_percent_edge_length() );
	dihedralPercentSpan1Input->set_parm_ptr( currGeom->get_dihedral_percent_edge_length() );
	dihedralDegPerSegInput->set_parm_ptr( currGeom->get_dihedral_degperseg() );

	hwbUI->rotateAirfoilButton->value( currGeom->get_dihed_rot_flag() );
	hwbUI->numInterpCounter->value( currGeom->get_num_interp() );

	hwbUI->sectionCounter->value( currGeom->get_curr_sect() );
	hwbUI->jointCounter->value( currGeom->get_curr_joint() );
	hwbUI->airfoilCounter->value( currGeom->get_curr_foil() );

	sprintf( str, "   %d", currGeom->get_num_sect() );
	hwbUI->totalNumSectOutput->value(str);


	//==== Airfoil Stuff ====//
	Af* afp = currGeom->get_af_ptr();
	hwbUI->airfoilTypeChoice->value( afp->get_type()-1 );
	hwbUI->airfoilInvertButton->value( afp->get_inverted_flag() );
	hwbUI->slatButton->value( afp->get_slat_flag() );
	hwbUI->slatShearButton->value( afp->get_slat_shear_flag() );
	hwbUI->flapButton->value( afp->get_flap_flag() );
	hwbUI->flapShearButton->value( afp->get_flap_shear_flag() );

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

	afSlatChordSlider->set_parm_ptr( afp->get_slat_chord() );
	afSlatChordInput->set_parm_ptr(  afp->get_slat_chord() );
	afSlatAngleSlider->set_parm_ptr( afp->get_slat_angle() );
	afSlatAngleInput->set_parm_ptr(  afp->get_slat_angle() );
	afFlapChordSlider->set_parm_ptr( afp->get_flap_chord() );
	afFlapChordInput->set_parm_ptr(  afp->get_flap_chord() );
	afFlapAngleSlider->set_parm_ptr( afp->get_flap_angle() );
	afFlapAngleInput->set_parm_ptr(  afp->get_flap_angle() );

	hwbUI->airfoilNameInput->value( afp->get_name() );

	if ( afp->get_type() == NACA_6_SERIES )
		hwbUI->sixSeriesChoice->activate();
	else
		hwbUI->sixSeriesChoice->deactivate();

	leRadiusInput->set_parm_ptr( afp->get_leading_edge_radius() );

	totalSpanButton->set_parm_ptr( currGeom->get_total_span() );
	totalProjSpanButton->set_parm_ptr( currGeom->get_total_proj_span() );
	totalChordButton->set_parm_ptr( currGeom->get_avg_chord() );
	totalAreaButton->set_parm_ptr( currGeom->get_total_area() );
	sweepOffsetButton->set_parm_ptr( currGeom->get_total_aspect() );


	glWin->setDrawBase( afp );
	glWin->redraw();

	hwbUI->UIWindow->show();
}

void HwbScreen::hide()
{
	hwbUI->UIWindow->hide();
}

void HwbScreen::position( int x, int y )
{
	hwbUI->UIWindow->position( x, y );
}

void HwbScreen::setTitle( const char* name )
{
	title = "HWB : ";
	title.concatenate( name );

	hwbUI->TitleBox->label( title );
}

void HwbScreen::parm_changed( Parm* parm )
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

void HwbScreen::updateName()
{
	Af* afp = currGeom->get_af_ptr();
	hwbUI->airfoilNameInput->value( afp->get_name() );

}

//==== Close Callbacks =====//
void HwbScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	hwbUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void HwbScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );
	if ( w == hwbUI->driverChoice )
	{
		currGeom->set_driver( hwbUI->driverChoice->value() );
		
		switch (currGeom->get_driver())
		{
			case AR_TR_A:
				scriptMgr->addLine("wing_sect", "driver", "artra");
				break;
			case AR_TR_S:
				scriptMgr->addLine("wing_sect", "driver", "artrs");
				break;
			case AR_TR_TC:
				scriptMgr->addLine("wing_sect", "driver", "artrtc");
				break;
			case AR_TR_RC:
				scriptMgr->addLine("wing_sect", "driver", "artrrc");
				break;
			case S_TC_RC:
				scriptMgr->addLine("wing_sect", "driver", "stcrc");
				break;
			case A_TC_RC:
				scriptMgr->addLine("wing_sect", "driver", "atcrc");
				break;
			case TR_S_A:
				scriptMgr->addLine("wing_sect", "driver", "trsa");
				break;
		}

	}
	else if ( w == hwbUI->airfoilTypeChoice )
	{
		int aftype = hwbUI->airfoilTypeChoice->value()+1;

		if ( aftype == AIRFOIL_FILE )
		{

//			char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
			char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");
			if ( newfile != NULL )
			{
				currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
				scriptMgr->addLine("wing_foil", "read", newfile);
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
			scriptMgr->addLine("wing_foil", "type", "naca4");
			break;
		case BICONVEX:
			scriptMgr->addLine("wing_foil", "type", "biconvex");
			break;
		case WEDGE:
			scriptMgr->addLine("wing_foil", "type", "wedge");
			break;
		case NACA_6_SERIES:
			scriptMgr->addLine("wing_foil", "type", "naca6");
			break;
		}
	}
	else if ( w ==  hwbUI->readFileButton )
	{
//		char *newfile = fl_file_chooser("Read Airfoil File?", "*.af", "");
		char *newfile = screenMgrPtr->FileChooser("Read Airfoil File?", "*.af");

		if ( newfile != NULL )
		{
			currGeom->get_af_ptr()->read_af_file( Stringc(newfile) );
			scriptMgr->addLine("wing_foil", "read", newfile);
		}
		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  hwbUI->sixSeriesChoice  )
	{
		int choice = hwbUI->sixSeriesChoice->value();

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
			scriptMgr->addLine("wing_foil", "6series", "63");
			break;
		case 1:
			scriptMgr->addLine("wing_foil", "6series", "64");
			break;
		case 2:
			scriptMgr->addLine("wing_foil", "6series", "65");
			break;
		case 3:
			scriptMgr->addLine("wing_foil", "6series", "66");
			break;
		case 4:
			scriptMgr->addLine("wing_foil", "6series", "67");
			break;
		case 5:
			scriptMgr->addLine("wing_foil", "6series", "63a");
			break;
		case 6:
			scriptMgr->addLine("wing_foil", "6series", "64a");
			break;
		case 7:
			scriptMgr->addLine("wing_foil", "6series", "65a");
			break;
		}
	}	

	else if ( w ==  hwbUI->sectionCounter  )
	{
		currGeom->set_curr_sect( (int)hwbUI->sectionCounter->value() );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_SECT );
		show(currGeom);
		scriptMgr->addLine("wing_sect", "select", currGeom->get_curr_sect());
	}
	else if ( w ==  hwbUI->jointCounter  )
	{
		currGeom->set_curr_joint( (int)hwbUI->jointCounter->value()  );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_JOINT );
	//	if((int)hwbUI->jointCounter->value() == 0)
	//		hwbUI->rotateAirfoilButton->deactivate();
	//	else
	//		hwbUI->rotateAirfoilButton->activate();
		show(currGeom);
		scriptMgr->addLine("wing_dihed", "select", currGeom->get_curr_joint());
	}
	else if ( w ==  hwbUI->airfoilCounter  )
	{
		currGeom->set_curr_foil( (int)hwbUI->airfoilCounter->value()  );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_FOIL );
		show(currGeom);
		scriptMgr->addLine("wing_foil", "select", currGeom->get_curr_foil());
	}
	else if ( w ==  hwbUI->addSectButton  )
	{
		currGeom->add_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect add");
	}
	else if ( w ==  hwbUI->insertSectButton  )
	{
		currGeom->ins_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect insert");
	}
	else if ( w ==  hwbUI->copySectButton  )
	{
		currGeom->copy_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect copy");
	}
	else if ( w ==  hwbUI->pasteSectButton  )
	{
		currGeom->paste_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect paste");
	}
	else if ( w ==  hwbUI->delSectButton  )
	{
		currGeom->del_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect del");
	}
	else if ( w ==  hwbUI->rotateAirfoilButton )
	{
		int val = hwbUI->rotateAirfoilButton->value();
		currGeom->set_dihed_rot_flag( val );
		currGeom->triggerDraw();
		scriptMgr->addLine("wing_dihed",  "rotfoil", val);
	}
	else if ( w ==  hwbUI->numInterpCounter  )
	{
		int val = (int)hwbUI->numInterpCounter->value();
		currGeom->set_num_interp( val );
		currGeom->triggerDraw();
		scriptMgr->addLine("wing_sect",  "num_interp", val);
	}
	else if ( w ==  hwbUI->incAllInterpButton   )
	{
		currGeom->inc_all_interp();
		currGeom->triggerDraw();
		show(currGeom);
		scriptMgr->addLine("wing_sect inc_all_interp");
	}
	else if ( w ==  hwbUI->decAllInterpButton   )
	{
		currGeom->dec_all_interp();
		currGeom->triggerDraw();
		show(currGeom);
		scriptMgr->addLine("wing_sect dec_all_interp");
	}
	else if ( w ==  hwbUI->MenuTabs   )
	{
		if ( hwbUI->MenuTabs->value() == hwbUI->filletGroup )
		{
			currGeom->set_curr_joint( (int)hwbUI->jointCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_JOINT );
			show(currGeom);
		}
		else if ( hwbUI->MenuTabs->value() == hwbUI->foilGroup )
		{
			currGeom->set_curr_foil( (int)hwbUI->airfoilCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_FOIL );
			show(currGeom);
		}
		else if ( hwbUI->MenuTabs->value() == hwbUI->sectGroup )
		{
			currGeom->set_curr_sect( (int)hwbUI->sectionCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_SECT );
			show(currGeom);
		}
		else if ( hwbUI->MenuTabs->value() == hwbUI->genGroup ||
			      hwbUI->MenuTabs->value() == hwbUI->xformGroup || 
				  hwbUI->MenuTabs->value() == hwbUI->planGroup )
		{
			currGeom->set_highlight_type( MSW_HIGHLIGHT_NONE );
			currGeom->triggerDraw();
		}

	}
	else if ( w ==  hwbUI->airfoilInvertButton   )
	{
		int flag = hwbUI->airfoilInvertButton->value();

		currGeom->get_af_ptr()->set_inverted_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

		scriptMgr->addLine("wing_foil", "inverty", flag);
	}

	else if ( w ==  hwbUI->slatButton   )
	{
		int flag = hwbUI->slatButton->value();
		currGeom->get_af_ptr()->set_slat_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  hwbUI->slatShearButton   )
	{
		int flag = hwbUI->slatShearButton->value();
		currGeom->get_af_ptr()->set_slat_shear_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  hwbUI->flapButton   )
	{
		int flag = hwbUI->flapButton->value();
		currGeom->get_af_ptr()->set_flap_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  hwbUI->flapShearButton   )
	{
		int flag = hwbUI->flapShearButton->value();
		currGeom->get_af_ptr()->set_flap_shear_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}

}
