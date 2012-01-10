//****************************************************************************
//    
//   Wing Screen Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "msWingScreen.h"
#include "materialMgr.h"
#include "screenMgr.h"
#include "af.h"
#include "curveEditScreen.h"
#include "scriptMgr.h"

//==== Constructor =====//
MsWingScreen::MsWingScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	MsWingUI* ui = msWingUI = new MsWingUI();

	msWingUI->UIWindow->position( 760, 30 );
	msWingUI->UIWindow->callback( staticCloseCB, this );

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

	msWingUI->driverChoice->callback( staticScreenCB, this );

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
	dihed2Slider = new Slider_adj_range(this, ui->dihed2Slider, ui->dihed2ButtonL, ui->dihed2ButtonR, 10.0 );
	dihed2Input = new Input( this, ui->dihed2Input );
	dihedCrv1Slider = new Slider_adj_range(this, ui->dihedCrv1Slider, ui->dihedCrv1ButtonL, ui->dihedCrv1ButtonR, 1.0 );
	dihedCrv1Input = new Input( this, ui->dihedCrv1Input );
	dihedCrv2Slider = new Slider_adj_range(this, ui->dihedCrv2Slider, ui->dihedCrv2ButtonL, ui->dihedCrv2ButtonR, 1.0 );
	dihedCrv2Input = new Input( this, ui->dihedCrv2Input );
	dihedCrv1StrSlider = new Slider_adj_range(this, ui->dihedCrv1StrSlider, ui->dihedCrv1StrButtonL, ui->dihedCrv1StrButtonR, 1.0 );
	dihedCrv1StrInput = new Input( this, ui->dihedCrv1StrInput );
	dihedCrv2StrSlider = new Slider_adj_range(this, ui->dihedCrv2StrSlider, ui->dihedCrv2StrButtonL, ui->dihedCrv2StrButtonR, 1.0 );
	dihedCrv2StrInput = new Input( this, ui->dihedCrv2StrInput );

	degPerSegInput = new Input( this, ui->degPerSegInput );
	degPerSegInput->set_format( "%1.0f" );
	maxNumSegsInput = new Input( this, ui->maxNumSegsInput );
	maxNumSegsInput->set_format( "%1.0f" );

	ui->rotateAirfoilButton->callback( staticScreenCB, this );
	ui->smoothBlendButton->callback( staticScreenCB, this );
	ui->numInterpCounter->callback( staticScreenCB, this );
	ui->incAllInterpButton->callback( staticScreenCB, this );
	ui->decAllInterpButton->callback( staticScreenCB, this );

	ui->relativeDihedralButton->callback( staticScreenCB, this );
	ui->relativeTwistButton->callback( staticScreenCB, this );
	ui->roundedTipsButton->callback( staticScreenCB, this );

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

	ARButton = new ParmButton( this, ui->ARButton );
	TRButton = new ParmButton( this, ui->TRButton );
	areaButton = new ParmButton( this, ui->areaButton );
	spanButton = new ParmButton( this, ui->spanButton );
	TCButton = new ParmButton( this, ui->TCButton );
	RCButton = new ParmButton( this, ui->RCButton );

	sweepButton = new ParmButton( this, ui->sweepButton );
	sweepLocButton = new ParmButton( this, ui->sweepLocButton );
	twistButton = new ParmButton( this, ui->twistButton );
	twistLocButton = new ParmButton( this, ui->twistLocButton );


	dihed1Button = new ParmButton( this, ui->dihed1Button );
	dihed2Button = new ParmButton( this, ui->dihed2Button );
	dihedCrv1Button = new ParmButton( this, ui->dihedCrv1Button );
	dihedCrv2Button = new ParmButton( this, ui->dihedCrv2Button );
	dihedCrv1StrButton = new ParmButton( this, ui->dihedCrv1StrButton );
	dihedCrv2StrButton = new ParmButton( this, ui->dihedCrv2StrButton );

	afCamberButton = new ParmButton( this, ui->camberButton );
	afCamberLocButton = new ParmButton( this, ui->camberLocButton );
	afThickButton = new ParmButton( this, ui->thickButton );
	afThickLocButton = new ParmButton( this, ui->thickLocButton );
	afIdealClButton = new ParmButton( this, ui->idealClButton );
	afAButton = new ParmButton( this, ui->aButton );

	afSlatChordButton = new ParmButton( this, ui->slatChordButton );
	afSlatAngleButton = new ParmButton( this, ui->slatAngleButton );
	afFlapChordButton = new ParmButton( this, ui->flapChordButton );
	afFlapAngleButton = new ParmButton( this, ui->flapAngleButton );


	ui->MenuTabs->callback( staticScreenCB, this );

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->airfoilGlWinGroup;
	ui->airfoilGlWinGroup->begin();
	glWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->airfoilGlWinGroup->end();

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();


}

//==== Destructor =====//
MsWingScreen::~MsWingScreen()
{
}

void MsWingScreen::show(Geom* geomPtr)
{
	char str[256];

	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (Ms_wing_geom*)geomPtr;

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

	msWingUI->driverChoice->value( currGeom->get_driver() ); 

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

	//==== Dihedral Stuff ====//
	dihed1Slider->set_parm_ptr( currGeom->get_sect_dihed1() );
	dihed1Input->set_parm_ptr( currGeom->get_sect_dihed1() );
	dihed2Slider->set_parm_ptr( currGeom->get_sect_dihed2() );
	dihed2Input->set_parm_ptr( currGeom->get_sect_dihed2() );

	dihedCrv1Slider->set_parm_ptr( currGeom->get_sect_dihed_crv1() );
	dihedCrv1Input->set_parm_ptr( currGeom->get_sect_dihed_crv1() );
	dihedCrv2Slider->set_parm_ptr( currGeom->get_sect_dihed_crv2() );
	dihedCrv2Input->set_parm_ptr( currGeom->get_sect_dihed_crv2() );

	dihedCrv1StrSlider->set_parm_ptr( currGeom->get_sect_dihed_crv1_str() );
	dihedCrv1StrInput->set_parm_ptr( currGeom->get_sect_dihed_crv1_str() );
	dihedCrv2StrSlider->set_parm_ptr( currGeom->get_sect_dihed_crv2_str() );
	dihedCrv2StrInput->set_parm_ptr( currGeom->get_sect_dihed_crv2_str() );

	degPerSegInput->set_parm_ptr( currGeom->get_deg_per_seg() );
	maxNumSegsInput->set_parm_ptr( currGeom->get_max_num_segs() );

	msWingUI->rotateAirfoilButton->value( currGeom->get_dihed_rot_flag() );
	msWingUI->smoothBlendButton->value( currGeom->get_smooth_blend_flag() );
	msWingUI->numInterpCounter->value( currGeom->get_num_interp() );

	msWingUI->relativeDihedralButton->value( currGeom->get_rel_dihedral_flag() );
	msWingUI->relativeTwistButton->value( currGeom->get_rel_twist_flag() );
	msWingUI->roundedTipsButton->value( currGeom->get_round_end_cap_flag() );

	msWingUI->sectionCounter->value( currGeom->get_curr_sect() );
	msWingUI->jointCounter->value( currGeom->get_curr_joint() );
	msWingUI->airfoilCounter->value( currGeom->get_curr_foil() );

	sprintf( str, "   %d", currGeom->get_num_sect() );
	msWingUI->totalNumSectOutput->value(str);


	//==== Airfoil Stuff ====//
	Af* afp = currGeom->get_af_ptr();
	msWingUI->airfoilTypeChoice->value( afp->get_type()-1 );
	msWingUI->airfoilInvertButton->value( afp->get_inverted_flag() );
	msWingUI->slatButton->value( afp->get_slat_flag() );
	msWingUI->slatShearButton->value( afp->get_slat_shear_flag() );
	msWingUI->flapButton->value( afp->get_flap_flag() );
	msWingUI->flapShearButton->value( afp->get_flap_shear_flag() );

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

	totalSpanButton->set_parm_ptr( currGeom->get_total_span() );
	totalProjSpanButton->set_parm_ptr( currGeom->get_total_proj_span() );
	totalChordButton->set_parm_ptr( currGeom->get_avg_chord() );
	totalAreaButton->set_parm_ptr( currGeom->get_total_area() );
	sweepOffsetButton->set_parm_ptr( currGeom->get_total_aspect() );

	WingSect* ws = currGeom->getCurrWingSect();

	ARButton->set_parm_ptr( ws->get_ar() );
	TRButton->set_parm_ptr( ws->get_tr() );
	areaButton->set_parm_ptr( ws->get_area() );
	spanButton->set_parm_ptr( ws->get_span() );
	TCButton->set_parm_ptr( ws->get_tc() );
	RCButton->set_parm_ptr( ws->get_rc() );

	sweepButton->set_parm_ptr( ws->get_sweep() );
	sweepLocButton->set_parm_ptr( ws->get_sweepLoc() );
	twistButton->set_parm_ptr( ws->get_twist() );
	twistLocButton->set_parm_ptr( ws->get_twistLoc()  );

	dihed1Button->set_parm_ptr( ws->get_dihedral() );
	dihed2Button->set_parm_ptr( ws->get_dihedral() );
	dihedCrv1Button->set_parm_ptr( ws->get_dihed_crv1() );
	dihedCrv2Button->set_parm_ptr( ws->get_dihed_crv2() );
	dihedCrv1StrButton->set_parm_ptr( ws->get_dihed_crv1_str() );
	dihedCrv2StrButton->set_parm_ptr( ws->get_dihed_crv2_str() );

	afCamberButton->set_parm_ptr( afp->get_camber() );
	afCamberLocButton->set_parm_ptr( afp->get_camber_loc() );
	afThickButton->set_parm_ptr( afp->get_thickness() );
	afThickLocButton->set_parm_ptr( afp->get_thickness_loc() );
	afIdealClButton->set_parm_ptr( afp->get_ideal_cl() );
	afAButton->set_parm_ptr( afp->get_a() );

	afSlatChordButton->set_parm_ptr( afp->get_slat_chord() );
	afSlatAngleButton->set_parm_ptr( afp->get_slat_angle() );
	afFlapChordButton->set_parm_ptr( afp->get_flap_chord() );
	afFlapAngleButton->set_parm_ptr( afp->get_flap_angle() );

	msWingUI->airfoilNameInput->value( afp->get_name() );

	if ( afp->get_type() == NACA_6_SERIES )
		msWingUI->sixSeriesChoice->activate();
	else
		msWingUI->sixSeriesChoice->deactivate();

	leRadiusInput->set_parm_ptr( afp->get_leading_edge_radius() );

	glWin->setDrawBase( afp );
	glWin->redraw();

	msWingUI->UIWindow->show();

}

void MsWingScreen::hide()
{
	msWingUI->UIWindow->hide();
}

void MsWingScreen::position( int x, int y )
{
	msWingUI->UIWindow->position( x, y );
}

void MsWingScreen::setTitle( const char* name )
{
	title = "MS_WING : ";
	title.concatenate( name );

	msWingUI->TitleBox->label( title );
}

void MsWingScreen::parm_changed( Parm* parm )
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

void MsWingScreen::updateName()
{
	Af* afp = currGeom->get_af_ptr();
	msWingUI->airfoilNameInput->value( afp->get_name() );

}

//==== Close Callbacks =====//
void MsWingScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	msWingUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void MsWingScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );
	if ( w == msWingUI->driverChoice )
	{
		currGeom->set_driver( msWingUI->driverChoice->value() );
		
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
	else if ( w == msWingUI->airfoilTypeChoice )
	{
		int aftype = msWingUI->airfoilTypeChoice->value()+1;

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
	else if ( w ==  msWingUI->readFileButton )
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
	else if ( w ==  msWingUI->sixSeriesChoice  )
	{
		int choice = msWingUI->sixSeriesChoice->value();

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

	else if ( w ==  msWingUI->sectionCounter  )
	{
		currGeom->set_curr_sect( (int)msWingUI->sectionCounter->value() );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_SECT );
		show(currGeom);
		scriptMgr->addLine("wing_sect", "select", currGeom->get_curr_sect());
	}
	else if ( w ==  msWingUI->jointCounter  )
	{
		currGeom->set_curr_joint( (int)msWingUI->jointCounter->value()  );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_JOINT );
		show(currGeom);
		scriptMgr->addLine("wing_dihed", "select", currGeom->get_curr_joint());
	}
	else if ( w ==  msWingUI->airfoilCounter  )
	{
		currGeom->set_curr_foil( (int)msWingUI->airfoilCounter->value()  );
		currGeom->set_highlight_type( MSW_HIGHLIGHT_FOIL );
		show(currGeom);
		scriptMgr->addLine("wing_foil", "select", currGeom->get_curr_foil());
	}
	else if ( w ==  msWingUI->addSectButton  )
	{
		currGeom->add_sect();
//		show(currGeom);
		scriptMgr->addLine("wing_sect add");

		//==== jrg - cheesy hack to fix slider bug - not sure what the problem is or why this fixes it ===//
		int cs = currGeom->get_curr_sect();
		currGeom->set_curr_sect( cs+1 );
		show(currGeom);
		currGeom->set_curr_sect( cs );
		show(currGeom);

	}
	else if ( w ==  msWingUI->insertSectButton  )
	{
		currGeom->ins_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect insert");

		//==== jrg - cheesy hack to fix slider bug - not sure what the problem is or why this fixes it ===//
		int cs = currGeom->get_curr_sect();
		currGeom->set_curr_sect( cs+1 );
		show(currGeom);
		currGeom->set_curr_sect( cs );
		show(currGeom);

	}
	else if ( w ==  msWingUI->copySectButton  )
	{
		currGeom->copy_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect copy");
	}
	else if ( w ==  msWingUI->pasteSectButton  )
	{
		currGeom->paste_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect paste");
	}
	else if ( w ==  msWingUI->delSectButton  )
	{
		currGeom->del_sect();
		show(currGeom);
		scriptMgr->addLine("wing_sect del");
	}
	else if ( w ==  msWingUI->rotateAirfoilButton  )
	{
		int val = msWingUI->rotateAirfoilButton->value();
		currGeom->set_dihed_rot_flag( val );
		currGeom->triggerDraw();
		scriptMgr->addLine("wing_dihed",  "rotfoil", val);
	}
	else if ( w ==  msWingUI->smoothBlendButton  )
	{
		int val = msWingUI->smoothBlendButton->value();
		currGeom->set_smooth_blend_flag( val );
		currGeom->triggerDraw();
		scriptMgr->addLine("wing_sect",  "smoothblend", val);
	}
	else if ( w ==  msWingUI->numInterpCounter  )
	{
		int val = (int)msWingUI->numInterpCounter->value();
		currGeom->set_num_interp( val );
		currGeom->triggerDraw();
		scriptMgr->addLine("wing_sect",  "num_interp", val);
	}
	else if ( w ==  msWingUI->incAllInterpButton   )
	{
		currGeom->inc_all_interp();
		currGeom->triggerDraw();
		show(currGeom);
		scriptMgr->addLine("wing_sect inc_all_interp");
	}
	else if ( w ==  msWingUI->decAllInterpButton   )
	{
		currGeom->dec_all_interp();
		currGeom->triggerDraw();
		show(currGeom);
		scriptMgr->addLine("wing_sect dec_all_interp");
	}
	else if ( w ==  msWingUI->MenuTabs   )
	{
		if ( msWingUI->MenuTabs->value() == msWingUI->dihedGroup )
		{
			currGeom->set_curr_joint( (int)msWingUI->jointCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_JOINT );
			show(currGeom);
		}
		else if ( msWingUI->MenuTabs->value() == msWingUI->foilGroup )
		{
			currGeom->set_curr_foil( (int)msWingUI->airfoilCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_FOIL );
			show(currGeom);
		}
		else if ( msWingUI->MenuTabs->value() == msWingUI->sectGroup )
		{
			currGeom->set_curr_sect( (int)msWingUI->sectionCounter->value()  );
			currGeom->set_highlight_type( MSW_HIGHLIGHT_SECT );
			show(currGeom);
		}
		else if ( msWingUI->MenuTabs->value() == msWingUI->genGroup ||
			      msWingUI->MenuTabs->value() == msWingUI->xformGroup || 
				  msWingUI->MenuTabs->value() == msWingUI->planGroup )
		{
			currGeom->set_highlight_type( MSW_HIGHLIGHT_NONE );
			currGeom->triggerDraw();
		}

	}
	else if ( w ==  msWingUI->airfoilInvertButton   )
	{
		int flag = msWingUI->airfoilInvertButton->value();

		currGeom->get_af_ptr()->set_inverted_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );

		scriptMgr->addLine("wing_foil", "inverty", flag);
	}
	else if ( w ==  msWingUI->slatButton   )
	{
		int flag = msWingUI->slatButton->value();
		currGeom->get_af_ptr()->set_slat_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  msWingUI->slatShearButton   )
	{
		int flag = msWingUI->slatShearButton->value();
		currGeom->get_af_ptr()->set_slat_shear_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  msWingUI->flapButton   )
	{
		int flag = msWingUI->flapButton->value();
		currGeom->get_af_ptr()->set_flap_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w ==  msWingUI->flapShearButton   )
	{
		int flag = msWingUI->flapShearButton->value();
		currGeom->get_af_ptr()->set_flap_shear_flag( flag );

		//==== Force Update and Redraw ===//
		currGeom->parm_changed( currGeom->get_af_ptr()->get_thickness() );
		show( currGeom );
	}
	else if ( w == msWingUI->relativeDihedralButton )
	{
		int flag = msWingUI->relativeDihedralButton->value();
		if ( flag )		currGeom->set_rel_dihedral_flag( true );
		else			currGeom->set_rel_dihedral_flag( false );
		currGeom->triggerDraw();
	}
	else if ( w == msWingUI->relativeTwistButton )
	{
		int flag = msWingUI->relativeTwistButton->value();
		if ( flag )		currGeom->set_rel_twist_flag( true );
		else			currGeom->set_rel_twist_flag( false );
		currGeom->triggerDraw();
	}
	else if ( w == msWingUI->roundedTipsButton )
	{
		int flag = msWingUI->roundedTipsButton->value();
		if ( flag )		currGeom->set_round_end_cap_flag( true );
		else			currGeom->set_round_end_cap_flag( false );
		currGeom->triggerDraw();
	}

}
