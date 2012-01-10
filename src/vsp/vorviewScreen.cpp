//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// labelScreen.cpp: implementation of the labelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "vorviewScreen.h"
#include "geomScreen.h"
#include "vorGeom.h"
#include "screenMgr.h"
#include "scriptMgr.h"
#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
InputValLink::InputValLink(double* v, Fl_Input* i)
{
	ival = 0;
	dval = v;
	input = i;
}

InputValLink::InputValLink(int* v, Fl_Input* i)
{
	dval = 0;
	ival = v;
	input = i;
}

void InputValLink::updateVal()
{
	if ( ival )
		*ival = atoi( input->value() );
	else if ( dval )
		*dval = atof(input->value());		
}

void InputValLink::updateInput()
{
	char str[256];
	if ( ival )
		sprintf( str, "  %d", *ival );
	else if ( dval )
		sprintf( str, "  %.3f", *dval );

	input->value( str );	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
InputVecLink::InputVecLink( vector<double*> v, Fl_Roller* roll, Fl_Button* lbut, Fl_Button* rbut, 
						    vector< Fl_Input*> ivec, vector< Fl_Button*> lvec )
{
	int i;
	dvalVec = v;
	leftButton  = lbut;
	rightButton = rbut;
	inputVec = ivec;
	labelVec = lvec;
	roller = roll;

	roller->callback( staticScreenCB, this );
	leftButton->callback( staticScreenCB, this );
	rightButton->callback( staticScreenCB, this );

	for ( i = 0 ; i < (int)labelVec.size() ; i++ )
	{
		labelStrVec.push_back( new char(16) );
	}

	for ( i = 0 ; i < (int)inputVec.size() ; i++ )
		inputVec[i]->callback( staticScreenCB, this );


	startIndex = 0;
}

void InputVecLink::screenCB( Fl_Widget* w )
{
	if ( w == leftButton )
	{
		startIndex--;
		if ( startIndex < 0 )
			startIndex = 0;
		updateInput();
	}
	else if ( w == rightButton )
	{
		startIndex++;
		int max = dvalVec.size() - inputVec.size()-1;
		if ( startIndex > max )
			startIndex = max;
		updateInput();
	}
	else if ( w == roller )
	{
		startIndex = (int)roller->value();
		if ( startIndex < 0 )
			startIndex = 0;
		int max = dvalVec.size() - inputVec.size()-1;
		if ( startIndex > max )
			startIndex = max;		
		updateInput();
	}
	else
	{
		updateVal();

	}


}


void InputVecLink::updateVal()
{
	for ( int i = 0 ; i < (int)inputVec.size() ; i++ )
	{
		double* dval = 0;
		if ( i+startIndex < (int)dvalVec.size() )
			dval = dvalVec[i+startIndex];

		if ( dval )
			*dval = atof(inputVec[i]->value());	
	}

}

void InputVecLink::updateInput()
{	
	char str[256];

	for ( int i = 0 ; i < (int)inputVec.size() ; i++ )
	{
		double* dval = 0;
		if ( i+startIndex < (int)dvalVec.size() )
			dval = dvalVec[i+startIndex];

		if (dval)
			sprintf( str, "  %.3f", *dval );
		else
			sprintf( str, "");

		inputVec[i]->value( str );
		
		sprintf( labelStrVec[i], "%d", i+startIndex+1 );
		labelVec[i]->label( labelStrVec[i] );

	}

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VorviewScreen::VorviewScreen(ScreenMgr* mgrPtr, Aircraft* airPtr, VspGlWindow* winPtr) : VspScreen( mgrPtr )
{
	int i;

	screenMgrPtr = mgrPtr;
	aircraftPtr = airPtr;
	glWinPtr = winPtr;

	//==== Set VorGeom Ptr - Only One ===//
	vorGeom = aircraftPtr->getVorGeom();
	vorGeom->setVorviewScreenPtr( this );

	vorviewUI = new VorviewUI();
	vorviewUI->UIWindow->position( 610, 30 );
	vorviewUI->UIWindow->callback( staticCloseCB, this );

	//==== Vorlax Output Screen ====//
	vorlaxOutUI = new VorlaxOutUI();
	vorlaxOutUI->UIWindow->position( 760, 400 );
	vorlaxOutUI->outputTextDisplay->buffer( new Fl_Text_Buffer() );

	//==== GUI Callbacks ===//
	vorviewUI->hrmFileButton->callback( staticScreenCB, this );
	vorviewUI->casFileButton->callback( staticScreenCB, this );
	vorviewUI->exportCasFileButton->callback( staticScreenCB, this );
		
	vorviewUI->displayCamberButton->callback( staticScreenCB, this );
	vorviewUI->displayCpButton->callback( staticScreenCB, this );
	vorviewUI->displayTurnOffAllButton->callback( staticScreenCB, this );
	vorviewUI->displaySlicesButton->callback( staticScreenCB, this );
	vorviewUI->displaySubPolysButton->callback( staticScreenCB, this );
	vorviewUI->displaySurfaceCpButton->callback( staticScreenCB, this );
	vorviewUI->displayXSecButton->callback( staticScreenCB, this );
	vorviewUI->displayCamberButton->callback( staticScreenCB, this );

	//==== Execute Stuff ====//
	vorviewUI->runVorlaxButton->callback( staticScreenCB, this );
	vorviewUI->subdivideButton->callback( staticScreenCB, this );
	vorviewUI->surfaceCpButton->callback( staticScreenCB, this );

	vorviewUI->autoFindButton->callback( staticScreenCB, this );
	vorviewUI->autoEliminateButton->callback( staticScreenCB, this );
	vorviewUI->autoMergeButton->callback( staticScreenCB, this );
	vorviewUI->autoSliceButton->callback( staticScreenCB, this );
	vorviewUI->autoAllButton->callback( staticScreenCB, this );
	vorviewUI->sliceResetButton->callback( staticScreenCB, this );

//	vorviewUI->sliceConstantYButton->callback( staticScreenCB, this );
//	vorviewUI->sliceConstantZButton->callback( staticScreenCB, this );
	vorviewUI->sliceSymButton->callback( staticScreenCB, this );
	vorviewUI->sliceFlatPlateButton->callback( staticScreenCB, this );

	//==== Value - Input Links - For .cas File Input===//
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.lax), vorviewUI->laxInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.hag), vorviewUI->hagInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.isweep), vorviewUI->isweepInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.itrmax), vorviewUI->itrmaxInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.idetail), vorviewUI->idetailInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.spc), vorviewUI->spcInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.floatx), vorviewUI->floatxInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.floaty), vorviewUI->floatyInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.nmach ), vorviewUI->nmachInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.nalpha ), vorviewUI->nalphaInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.latral ), vorviewUI->latralInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.psi ), vorviewUI->psiInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.pitchq ), vorviewUI->pitchqInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.rollq ), vorviewUI->rollqInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.yawq ), vorviewUI->yawqInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.vinf ), vorviewUI->vinfInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.sref ), vorviewUI->srefInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ar ),   vorviewUI->arInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.taper ), vorviewUI->taperInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.cbar ), vorviewUI->cbarInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.xbar ), vorviewUI->xbarInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.zbar ), vorviewUI->zbarInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.wspan ), vorviewUI->wspanInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ltail ),  vorviewUI->ltailInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ztail ),  vorviewUI->ztailInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.swpwng ), vorviewUI->swpwngInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.istab ),  vorviewUI->istabInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.iaero ),  vorviewUI->iaeroInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.cdmach ), vorviewUI->cdmachInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.cdo ),    vorviewUI->cdoInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.iburst ),    vorviewUI->iburstInput  ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.xref ),  vorviewUI->xrefInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.angcrt ),  vorviewUI->angcrtInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ck1 ),  vorviewUI->ck1Input ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ck2 ),  vorviewUI->ck2Input ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.iac ),  vorviewUI->iacInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.aceps ),  vorviewUI->acepsInput ) );

	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.itrim ),  vorviewUI->itrimInput  ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.ntrim ),  vorviewUI->ntrimInput   ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.cmeps ),  vorviewUI->cmepsInput   ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.reinf  ),  vorviewUI->reinfInput  ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.reflen ),  vorviewUI->reflenInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.idrag  ),  vorviewUI->idragInput ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.numren  ),  vorviewUI->numrenInput  ) );
	ivLinkVec.push_back( new InputValLink( &(vorGeom->caseData.nummch  ),  vorviewUI->nummchInput ) );


	//==== Set Callbacks for Value - Input Links ===//
	for ( i = 0 ; i < (int)ivLinkVec.size() ; i++ )
	{
		ivLinkVec[i]->input->callback( staticScreenCB, this );
	}


	//==== Mach Array Input ===//
	vector<double*> dvec;
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		dvec.push_back( &vorGeom->caseData.xmach[i] );

	vector<Fl_Input*> inputvec;
	inputvec.push_back( vorviewUI->mach1Input );
	inputvec.push_back( vorviewUI->mach2Input );
	inputvec.push_back( vorviewUI->mach3Input );
	inputvec.push_back( vorviewUI->mach4Input );
	inputvec.push_back( vorviewUI->mach5Input );

	vector<Fl_Button*> labelvec;
	labelvec.push_back( vorviewUI->mach1Button );
	labelvec.push_back( vorviewUI->mach2Button );
	labelvec.push_back( vorviewUI->mach3Button );
	labelvec.push_back( vorviewUI->mach4Button );
	labelvec.push_back( vorviewUI->mach5Button );

	machNumLink = new InputVecLink( dvec, vorviewUI->machRoller, vorviewUI->machLeftButton, vorviewUI->machRightButton,
									inputvec, labelvec );

	//==== Alpha Array Input ===//
	inputvec.clear();
	inputvec.push_back( vorviewUI->alpha1Input  );
	inputvec.push_back( vorviewUI->alpha2Input );
	inputvec.push_back( vorviewUI->alpha3Input );
	inputvec.push_back( vorviewUI->alpha4Input );
	inputvec.push_back( vorviewUI->alpha5Input );

	labelvec.clear();
	labelvec.push_back( vorviewUI->alpha1Button );
	labelvec.push_back( vorviewUI->alpha2Button );
	labelvec.push_back( vorviewUI->alpha3Button );
	labelvec.push_back( vorviewUI->alpha4Button );
	labelvec.push_back( vorviewUI->alpha5Button );

	dvec.clear();
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		dvec.push_back( &vorGeom->caseData.alpha[i] );

	alphaNumLink = new InputVecLink( dvec, vorviewUI->alphaRoller, vorviewUI->alphaLeftButton, vorviewUI->alphaRightButton,
									inputvec, labelvec );

	//==== Alpha Trim Array Input ===//
	inputvec.clear();
	inputvec.push_back( vorviewUI->atrim1Input   );
	inputvec.push_back( vorviewUI->atrim2Input );
	inputvec.push_back( vorviewUI->atrim3Input );
	inputvec.push_back( vorviewUI->atrim4Input );
	inputvec.push_back( vorviewUI->atrim5Input );

	labelvec.clear();
	labelvec.push_back( vorviewUI->atrim1Button );
	labelvec.push_back( vorviewUI->atrim2Button );
	labelvec.push_back( vorviewUI->atrim3Button );
	labelvec.push_back( vorviewUI->atrim4Button );
	labelvec.push_back( vorviewUI->atrim5Button );

	dvec.clear();
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		dvec.push_back( &vorGeom->caseData.alpha_trim[i] );

	atrimNumLink = new InputVecLink( dvec, vorviewUI->atrimRoller, vorviewUI->atrimLeftButton, vorviewUI->atrimRightButton,
									inputvec, labelvec );

	//==== Cl Trim Array Input ===//
	inputvec.clear();
	inputvec.push_back( vorviewUI->cltrim1Input   );
	inputvec.push_back( vorviewUI->cltrim2Input );
	inputvec.push_back( vorviewUI->cltrim3Input );
	inputvec.push_back( vorviewUI->cltrim4Input );
	inputvec.push_back( vorviewUI->cltrim5Input );

	labelvec.clear();
	labelvec.push_back( vorviewUI->cltrim1Button );
	labelvec.push_back( vorviewUI->cltrim2Button );
	labelvec.push_back( vorviewUI->cltrim3Button );
	labelvec.push_back( vorviewUI->cltrim4Button );
	labelvec.push_back( vorviewUI->cltrim5Button );

	dvec.clear();
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		dvec.push_back( &vorGeom->caseData.cltrim[i] );

	cltrimNumLink = new InputVecLink( dvec, vorviewUI->cltrimRoller, vorviewUI->cltrimLeftButton, vorviewUI->cltrimRightButton,
									inputvec, labelvec );

	//==== Cm Trim Array Input ===//
	inputvec.clear();
	inputvec.push_back( vorviewUI->cmtrim1Input   );
	inputvec.push_back( vorviewUI->cmtrim2Input );
	inputvec.push_back( vorviewUI->cmtrim3Input );
	inputvec.push_back( vorviewUI->cmtrim4Input );
	inputvec.push_back( vorviewUI->cmtrim5Input );

	labelvec.clear();
	labelvec.push_back( vorviewUI->cmtrim1Button );
	labelvec.push_back( vorviewUI->cmtrim2Button );
	labelvec.push_back( vorviewUI->cmtrim3Button );
	labelvec.push_back( vorviewUI->cmtrim4Button );
	labelvec.push_back( vorviewUI->cmtrim5Button );

	dvec.clear();
	for ( i = 0 ; i < MAX_NUM_VEC ; i++ )
		dvec.push_back( &vorGeom->caseData.cmtrim[i] );

	cmtrimNumLink = new InputVecLink( dvec, vorviewUI->cmtrimRoller, vorviewUI->cmtrimLeftButton, vorviewUI->cmtrimRightButton,
									inputvec, labelvec );

	vorviewUI->camberScaleRoller->callback( staticScreenCB, this );

	//==== Cp Clamps ====//
	cpMinClampVal = 0;
	cpMaxClampVal = 0;
	vorviewUI->cpMaxClampRoller->callback( staticScreenCB, this );
	vorviewUI->cpMinClampRoller->callback( staticScreenCB, this );

	//==== Set Cp Colors ====//
	vec3d col = vorGeom->getCpColor( -0.01 );
	vorviewUI->cpMinColorBox->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 1.01 );
	vorviewUI->cpMaxColorBox->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 0.01 );
	vorviewUI->cpColorBox1->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 1.0/9.0 );
	vorviewUI->cpColorBox2->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 2.0/9.0 );
	vorviewUI->cpColorBox3->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 3.0/9.0 );
	vorviewUI->cpColorBox4->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 4.0/9.0 );
	vorviewUI->cpColorBox5->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 5.0/9.0 );
	vorviewUI->cpColorBox6->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 6.0/9.0 );
	vorviewUI->cpColorBox7->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 7.0/9.0 );
	vorviewUI->cpColorBox8->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 8.0/9.0 );
	vorviewUI->cpColorBox9->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));
	col = vorGeom->getCpColor( 0.99 );
	vorviewUI->cpColorBox10->color( fl_rgb_color( (int)col.x(), (int)col.y(), (int)col.z() ));

	//==== Number of Subpanels ====//
	vorviewUI->numSubPanelCounter->callback( staticScreenCB, this );
	vorviewUI->numSubPanelCounter->lstep(200.0);

	//vorviewUI->keySliceYPosSlider->callback( staticScreenCB, this );
	vorviewUI->keySliceYPosLeftButton->callback( staticScreenCB, this );
	vorviewUI->keySliceYPosRightButton->callback( staticScreenCB, this );

	vorviewUI->enableKeySliceButton->callback( staticScreenCB, this );

	vorviewUI->addKeySliceButton->callback( staticScreenCB, this );

	vorviewUI->controlAngInput->callback( staticScreenCB, this );
	vorviewUI->controlBrowser->callback( staticScreenCB, this );
	vorviewUI->controlChordInput->callback( staticScreenCB, this );
	vorviewUI->controlHingeInput->callback( staticScreenCB, this );
	vorviewUI->controlNameInput->callback( staticScreenCB, this );
	vorviewUI->controlReflChoice->callback( staticScreenCB, this );
	vorviewUI->controlSymChoice->callback( staticScreenCB, this );
	vorviewUI->controlTypeChoice->callback( staticScreenCB, this );

	vorviewUI->trapCounter->callback( staticScreenCB, this );
	vorviewUI->addControlButton->callback( staticScreenCB, this );
	vorviewUI->delControlButton->callback( staticScreenCB, this );

	vorviewUI->controlReflChoice->add("No");
	vorviewUI->controlReflChoice->add("Yes");

	vorviewUI->controlSymChoice->add("ASym");
	vorviewUI->controlSymChoice->add("Sym");

	vorviewUI->controlTypeChoice->add("Flap");
	vorviewUI->controlTypeChoice->add("Slat");
	vorviewUI->controlTypeChoice->add("All");
	vorviewUI->controlTypeChoice->add("Camb=0");

	vorviewUI->MenuTabs->callback( staticScreenCB, this );
	vorviewUI->addTrapControlButton->callback( staticScreenCB, this );

	m_KeySliceYPosSlider = new SliderInputCombo( vorviewUI->keySliceYPosSlider,  vorviewUI->keySliceYPosInput );
	m_KeySliceYPosSlider->SetCallback( staticScreenCB, this );
	m_KeySliceYPosSlider->Init( 0.0, 1.0, 0.5, 1.0 );

	vorviewUI->editKeySliceSelector->callback( staticScreenCB, this );
	vorviewUI->editKeySliceDeleteButton->callback( staticScreenCB, this );

	m_EditKeySliceYPosSlider = new SliderInputCombo( vorviewUI->editKeySliceYPosSlider,  vorviewUI->editKeySliceYPosInput );
	m_EditKeySliceYPosSlider->SetCallback( staticScreenCB, this );
	m_EditKeySliceYPosSlider->Init( 0.0, 1.0, 0.5, 1.0 );

	vorviewUI->editKeySliceYPosRightButton->callback( staticScreenCB, this );
	vorviewUI->editKeySliceYPosLeftButton->callback( staticScreenCB, this );

	vorviewUI->numRegionSlices->callback( staticScreenCB, this );
	vorviewUI->numRegionSlices->step( 1, 5 );


}

void VorviewScreen::screenCB( Fl_Widget* w )
{
	int i;
	if ( w == vorviewUI->hrmFileButton )
	{
//		char* newfile = fl_file_chooser("Read Cross Sect File?", "*.hrm", "");	//jrg add script stuff
		char* newfile = screenMgrPtr->FileChooser("Read Cross Sect File?", "*.hrm");	//jrg add script stuff
		if ( newfile )
			vorGeom->read_xsec(newfile);

		aircraftPtr->update_bbox();
		aircraftPtr->triggerDraw();

//		if (vorGeom )
//		{
//			aircraftPtr->setActiveGeom( vorGeom );
//			screenMgrPtr->getGeomScreen()->deselectGeomBrowser();
//			screenMgrPtr->update( GEOM_SCREEN );
//		}

		update();
		screenMgrPtr->getDrawWin()->centerAllViews();
	}
	else if ( w == vorviewUI->casFileButton )
	{
//		char* newfile = fl_file_chooser("Import Case File?", "*.cas", "");
		char* newfile = screenMgrPtr->FileChooser("Import Case File?", "*.cas");
		if ( newfile )
		{
			vorGeom->caseData.read_file( newfile );
			update();
		}
	}
	else if ( w == vorviewUI->exportCasFileButton )
	{
//		char* newfile = fl_file_chooser("Export Case File?", "*.cas", "");
		char* newfile = screenMgrPtr->FileChooser("Export Case File?", "*.cas");
		if ( newfile )
		{
			vorGeom->caseData.write_file( newfile );
		}
	}

/***********
	else if ( w == vorviewUI->importVSPGeomButton )
	{
		//==== Count Num Comps Not Counting vorGeom ====//
		int numc = 0;
		for ( i = 0 ; i < aircraftPtr->getGeomVec().size() ; i++ )
		{
			if ( aircraftPtr->getGeomVec()[i] != vorGeom )
				numc++;
		}

		if ( numc >= 1 )
		{
			Stringc fn = vorGeom->getBaseName();
			fn.concatenate( Stringc(".hrm") );
			aircraftPtr->write_xsec_file( fn.get_char_star() );

			for ( i = 0 ; i < aircraftPtr->getGeomVec().size() ; i++ )
			{
				aircraftPtr->getGeomVec()[i]->setNoShowFlag(1);
			}
			vorGeom = (VorGeom*)aircraftPtr->read_xsec_file( fn );
		}
		screenMgrPtr->update( GEOM_SCREEN );
	}
***********/

	else if ( w == vorviewUI->displayCamberButton )
	{
		vorGeom->setDrawFlag(DRAW_CAMBER, vorviewUI->displayCamberButton->value() );
	}
	else if ( w == vorviewUI->displayTurnOffAllButton )
	{
		for ( i = 0 ; i < NUM_DRAW_FLAGS ; i++ )
		{
			vorGeom->setDrawFlag( i, 0 );
		}
		update();

		//==== Show/NoShow Geom ====//
		for ( i = 0 ; i < (int)aircraftPtr->getGeomVec().size() ; i++ )
		{
			if ( aircraftPtr->getGeomVec()[i] != vorGeom )
			{
				if ( vorGeom->getDrawFlag(DRAW_XSEC) )
					aircraftPtr->getGeomVec()[i]->setNoShowFlag(0);
				else
					aircraftPtr->getGeomVec()[i]->setNoShowFlag(1);
			}
		}
		screenMgrPtr->update( GEOM_SCREEN );

	}
	else if ( w == vorviewUI->displaySlicesButton )
	{
		vorGeom->setDrawFlag(DRAW_SLICE, vorviewUI->displaySlicesButton->value() );
	}
	else if ( w == vorviewUI->displaySubPolysButton )
	{
		vorGeom->setDrawFlag(DRAW_SUBPOLY, vorviewUI->displaySubPolysButton->value() );
	}
	else if ( w == vorviewUI->displayCpButton )
	{
		vorGeom->setDrawFlag(DRAW_CP, vorviewUI->displayCpButton->value() );

		//==== Turn Off Surf Cp Flag ====//
		if ( vorGeom->getDrawFlag(DRAW_CP) )
		{
			vorGeom->setDrawFlag(DRAW_CP_SURF, 0 );
            vorviewUI->displaySurfaceCpButton->value(0);
		}
		update();
	}
	else if ( w == vorviewUI->displaySurfaceCpButton )
	{
		vorGeom->setDrawFlag(DRAW_CP_SURF, vorviewUI->displaySurfaceCpButton->value() );

		//==== Turn Off Cp Flag ====//
		if ( vorGeom->getDrawFlag(DRAW_CP_SURF) )
		{
			vorGeom->setDrawFlag(DRAW_CP, 0 );
            vorviewUI->displayCpButton->value(0);
		}
		update();
	}
	else if ( w == vorviewUI->displayXSecButton )
	{
		vorGeom->setDrawFlag(DRAW_XSEC, vorviewUI->displayXSecButton->value() );

		//==== Show/NoShow Geom ====//
		for ( i = 0 ; i < (int)aircraftPtr->getGeomVec().size() ; i++ )
		{
			if ( aircraftPtr->getGeomVec()[i] != vorGeom )
			{
				if ( vorGeom->getDrawFlag(DRAW_XSEC) )
					aircraftPtr->getGeomVec()[i]->setNoShowFlag(0);
				else
					aircraftPtr->getGeomVec()[i]->setNoShowFlag(1);
			}
		}
		screenMgrPtr->update( GEOM_SCREEN );
	}
	else if ( w == vorviewUI->runVorlaxButton )
	{
/*
		if ( vorGeom->forkVorlax(EXECUTE_VORLAX) )
		{
			Fl::add_idle(staticRunVorlaxCB, this); 
		}
*/			
		m_DisplayResults = true;


		vorGeom->readInputVorlaxFiles();
		vorGeom->runVorlax(EXECUTE_VORLAX);

		vorGeom->readVorlaxFiles();
		update();
		aircraftPtr->triggerDraw();

		Stringc outfn = vorGeom->getBaseName();
		outfn.concatenate( Stringc(".out") );

		vorlaxOutUI->UIWindow->show();
		vorlaxOutUI->outputTextDisplay->buffer()->loadfile(outfn.get_char_star());

		vorGeom->setDrawFlag(DRAW_CP, 1 );
		vorviewUI->displayCpButton->value(1);
		//==== Turn Off Surf Cp Flag ====//
		if ( vorGeom->getDrawFlag(DRAW_CP) )
		{
			vorGeom->setDrawFlag(DRAW_CP_SURF, 0 );
            vorviewUI->displaySurfaceCpButton->value(0);
		}

		vorviewUI->executeOutput->value("Done..");

		update();

	}
	else if ( w == vorviewUI->subdivideButton  )
	{
		m_DisplayResults = false;
		vorGeom->readInputVorlaxFiles();
	    if ( vorGeom->forkVorlax(EXECUTE_SUBDIVIDE) )
		{
			Fl::add_idle(staticRunVorlaxCB, this);
		}
		vorGeom->setDrawFlag(DRAW_SUBPOLY, 1);
		vorviewUI->displaySubPolysButton->value(1);
	}
	else if ( w == vorviewUI->surfaceCpButton  )
	{
		m_DisplayResults = true;
		vorGeom->readInputVorlaxFiles();
		if ( vorGeom->forkVorlax(EXECUTE_SURFACE_CP) )
		{
			Fl::add_idle(staticRunVorlaxCB, this); 
		}

		vorGeom->setDrawFlag(DRAW_CP_SURF, 1 );
		vorviewUI->displaySurfaceCpButton->value(1);
		//==== Turn Off Cp Flag ====//
		if ( vorGeom->getDrawFlag(DRAW_CP_SURF) )
		{
			vorGeom->setDrawFlag(DRAW_CP, 0 );
            vorviewUI->displayCpButton->value(0);
		}
		update();
	}
	else if ( w == vorviewUI->cpMaxClampRoller  )
	{
		int val = (int)vorviewUI->cpMaxClampRoller->value();
		int del = val - cpMaxClampVal;

		vorGeom->incCpClamps( 0, del );
		cpMaxClampVal = val;
		update();
	}
	else if ( w == vorviewUI->cpMinClampRoller  )
	{
		int val = (int)vorviewUI->cpMinClampRoller->value();
		int del = val - cpMinClampVal;

		vorGeom->incCpClamps( del,0 );
		cpMinClampVal = val;
		update();
	}
	else if ( w == vorviewUI->camberScaleRoller  )
	{
		double val = vorviewUI->camberScaleRoller->value();
		vorGeom->setCamberScale( val );
	}
	else if ( w == vorviewUI->numSubPanelCounter   )
	{
		int val = (int)vorviewUI->numSubPanelCounter->value();
		vorGeom->setNumSubPanels(val);
	}
	else if ( w == vorviewUI->autoFindButton   )
	{
		vorGeom->autoFind();
		vorGeom->setDrawFlag(DRAW_SLICE, 1);			// Turn On Slices
		vorviewUI->displaySlicesButton->value(1);
	}
	else if ( w == vorviewUI->autoEliminateButton   )
	{
		vorGeom->autoEliminate();
	}
	else if ( w == vorviewUI->autoMergeButton   )
	{
		vorGeom->autoMerge();
	}
	else if ( w == vorviewUI->autoSliceButton   )
	{
		vorGeom->autoSlice();
	}
	else if ( w == vorviewUI->autoAllButton   )
	{
		vorGeom->autoAll();
	}
	else if ( w == vorviewUI->sliceResetButton   )
	{
		vorGeom->autoReset();
	}
	else if ( w == vorviewUI->sliceSymButton   )
	{
		vorGeom->setKeySliceSymFlag( vorviewUI->sliceSymButton->value() );
	}
	else if ( m_KeySliceYPosSlider->GuiChanged( w ) )
	{
		double val = m_KeySliceYPosSlider->GetVal();
		vorGeom->setKeySliceY( (float)val );
		vorviewUI->enableKeySliceButton->value( vorGeom->getKeySliceTempFlag() );
	}
	else if ( w == vorviewUI->keySliceYPosLeftButton   )
	{
		vorGeom->adjustKeySliceY( -1.0 );
		m_KeySliceYPosSlider->SetVal( vorGeom->getTempKeySliceY() );
		m_KeySliceYPosSlider->UpdateGui();
	}
	else if ( w == vorviewUI->keySliceYPosRightButton   )
	{
		vorGeom->adjustKeySliceY( 1.0 );	
		m_KeySliceYPosSlider->SetVal( vorGeom->getTempKeySliceY() );
		m_KeySliceYPosSlider->UpdateGui();
	}
	else if ( w == vorviewUI->enableKeySliceButton   )
	{
		vorGeom->setKeySliceTempFlag( vorviewUI->enableKeySliceButton->value() );
	}
	else if ( w == vorviewUI->addKeySliceButton   )
	{
		vorGeom->addKeySlice();
		vorviewUI->enableKeySliceButton->value( vorGeom->getKeySliceTempFlag() );
		update();
	}
	else if ( w == vorviewUI->editKeySliceSelector )
	{
		int id = (int)(vorviewUI->editKeySliceSelector->value()+0.01);
		vorGeom->setCurrKeySlice( id );
		update();
	}
	else if ( w == vorviewUI->editKeySliceDeleteButton )
	{
		vorGeom->delCurrKeySlice();
		update();
	}
	else if ( m_EditKeySliceYPosSlider->GuiChanged( w ) )
	{
		double val = m_EditKeySliceYPosSlider->GetVal();
		int sym = vorGeom->getKeySliceSymFlag();
		vorGeom->setEditKeySliceYFract( (float)val, sym );
	}
	else if ( w == vorviewUI->editKeySliceYPosLeftButton   )
	{
		double val = m_EditKeySliceYPosSlider->GetVal();
		int sym = vorGeom->getKeySliceSymFlag();
		vorGeom->setEditKeySliceYFract( (float)val - 0.001f, sym );
		update(); 
	}
	else if ( w == vorviewUI->editKeySliceYPosRightButton    )
	{
		double val = m_EditKeySliceYPosSlider->GetVal();
		int sym = vorGeom->getKeySliceSymFlag();
		vorGeom->setEditKeySliceYFract( (float)val + 0.001f, sym );
		update(); 
	}
	else if ( w == vorviewUI->numRegionSlices    )
	{
		int n = (int)vorviewUI->numRegionSlices->value();
		vorGeom->setCurrNumRegionSlices( n );
	}
	else if ( w == vorviewUI->sliceFlatPlateButton   )
	{
		vorGeom->setFlatFlag( vorviewUI->sliceFlatPlateButton->value() );
	}
//Control Stuff
	else if ( w == vorviewUI->trapCounter )
	{
		vorGeom->getVorSlicePtr()->set_curr_trap( (int)vorviewUI->trapCounter->value() );
	}
	else if ( w == vorviewUI->addControlButton )
	{
		vorGeom->getVorSlicePtr()->add_csf();
		vorGeom->writeCsf();

		update();
	}
	else if ( w == vorviewUI->delControlButton  )
	{
		vorGeom->getVorSlicePtr()->del_csf();
		vorGeom->writeCsf();

		update();
	}
	else if ( w == vorviewUI->controlBrowser  )
	{
		int val = vorviewUI->controlBrowser->value() - 1;
		
		vorGeom->getVorSlicePtr()->set_curr_csf(val);
		update();
	}
	else if ( w == vorviewUI->MenuTabs )
	{
		Fl_Widget * tab = vorviewUI->MenuTabs->value();

		if ( tab == vorviewUI->controlTab )
		{
			vorGeom->displayControl( 1 );
		}
		else
		{
			vorGeom->displayControl( 0 );
		}
	}
	else if ( w == vorviewUI->addTrapControlButton )
	{
		int ct = vorGeom->getVorSlicePtr()->get_curr_trap();
		int cc = vorGeom->getVorSlicePtr()->get_curr_csf();

		vorGeom->getVorSlicePtr()->add_del_csf_trap( cc, ct );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlAngInput )
	{
		double ang = atof(vorviewUI->controlAngInput->value());

		VOROBJ* VorPtr = vorGeom->getVorSlicePtr()->Vor;
		int curr_csf = VorPtr->curr_csf;

		if ( curr_csf >= 0 && curr_csf < VorPtr->ntotcsf )
		{
			VorPtr->csfdefangle[curr_csf] = (float)ang;
		}
		vorGeom->writeCsf();
	}

	else if ( w == vorviewUI->controlChordInput )
	{
		vorGeom->setCsfChord( atof(vorviewUI->controlChordInput->value()) );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlHingeInput )
	{
		vorGeom->setCsfHinge( atof(vorviewUI->controlHingeInput->value()) );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlAngInput )
	{
		vorGeom->setCsfAngle( atof(vorviewUI->controlAngInput->value()) );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlReflChoice )
	{
		vorGeom->setCsfRefl( vorviewUI->controlReflChoice->value() );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlSymChoice )
	{
		vorGeom->setCsfSym( vorviewUI->controlSymChoice->value() );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlTypeChoice )
	{
		vorGeom->setCsfType( vorviewUI->controlTypeChoice->value() );
		vorGeom->writeCsf();
	}
	else if ( w == vorviewUI->controlNameInput )
	{
		vorGeom->setCsfName( vorviewUI->controlNameInput->value() );
		vorGeom->writeCsf();
	}
	//==== Input Value Link Callbacks ====//
	else
	{
		for ( i = 0 ; i < (int)ivLinkVec.size() ; i++ )
		{
			if ( w == ivLinkVec[i]->input )
				ivLinkVec[i]->updateVal();
		}
	}

	aircraftPtr->triggerDraw();
}

void VorviewScreen::runVorlaxCB()
{
	//===== Animate Text While Executing ====//
	static int scnt = 0;
	scnt++;
	if ( scnt%6 == 0)
		vorviewUI->executeOutput->value("Executing.");
	else if ( scnt%6 == 2 )
		vorviewUI->executeOutput->value("Executing.....");
	else if ( scnt%6 == 4 )
		vorviewUI->executeOutput->value("Executing..........");

	//===== Check On Process Progress ====//
	double percentDone = 0.0;
	int runningFlag = vorGeom->checkVorlaxRun( &percentDone );

	if ( !runningFlag )
	{
		Fl::remove_idle(staticRunVorlaxCB,this);		// Remove This (Idle) Callback
			
		vorviewUI->executeOutput->value("");			// Set Progress to Zero
		vorviewUI->executeProgress->value(0);

		vorGeom->readVorlaxFiles();
		update();
		aircraftPtr->triggerDraw();

		Stringc outfn = vorGeom->getBaseName();
		outfn.concatenate( Stringc(".out") );

		if ( m_DisplayResults )
		{
			vorlaxOutUI->UIWindow->show();
			vorlaxOutUI->outputTextDisplay->buffer()->loadfile(outfn.get_char_star());
		}

	}
	else
	{
		vorviewUI->executeProgress->minimum(0.0f);
		vorviewUI->executeProgress->maximum(1.0);
		vorviewUI->executeProgress->value((float)percentDone);
	}
}


void VorviewScreen::updateVorlaxProgress( double fract )
{
	static int scnt = 0;
	scnt++;
	if ( scnt%6 == 0)
		vorviewUI->executeOutput->value("Executing.");
	else if ( scnt%6 == 2 )
		vorviewUI->executeOutput->value("Executing.....");
	else if ( scnt%6 == 4 )
		vorviewUI->executeOutput->value("Executing..........");

	vorviewUI->executeProgress->minimum(0.0f);
	vorviewUI->executeProgress->maximum(1.0);
	vorviewUI->executeProgress->value((float)fract);
		
	vorlaxOutUI->UIWindow->show();

}


void VorviewScreen::show(Geom*)
{
	if (!vorviewUI) 
		return;

	//==== If There Are Some Geom Comps ====//
	if ( aircraftPtr->getGeomVec().size())
	{

		//==== Extract Base Name ====//
		Stringc bName = aircraftPtr->getFileName();			
		bName.remove_substring(".vsp", 1);

		vorGeom->setBaseName( bName );
		vorGeom->convertToHerm();
		vorGeom->readVorlaxFiles();
	}
	vorGeom->setNoShowFlag(0);

	update();

	vorviewUI->UIWindow->show(); 
}


void VorviewScreen::update()
{
	//==== Title Strings ====//
	static char str1[255], str2[255];

	//==== Strip out Path ====//
	Stringc full_path_name = vorGeom->getBaseName();
	full_path_name.change_from_to('\\', ' ');
	full_path_name.change_from_to('/', ' ');
	titleName = full_path_name.get_word( full_path_name.count_words()-1 );

	strcpy(str1, titleName.get_char_star() );		// Need static copy	
	vorviewUI->hrmFileBox->label( str1 );

	sprintf(str2, "VORVIEW : %s", titleName.get_char_star() );
	vorviewUI->TitleBox->label( str2 );


	//==== Update All Linked Values ====//
	for ( int i = 0 ; i < (int)ivLinkVec.size() ; i++ )
	{
		ivLinkVec[i]->updateInput();
	}
	machNumLink->updateInput();
	alphaNumLink->updateInput();
	atrimNumLink->updateInput();
	cltrimNumLink->updateInput();
	cmtrimNumLink->updateInput();

	//==== Update Draw Show Buttons ====//
	vorviewUI->displayCpButton->value(vorGeom->getDrawFlag(DRAW_CP));
	vorviewUI->displaySlicesButton->value(vorGeom->getDrawFlag(DRAW_SLICE));
	vorviewUI->displaySubPolysButton->value(vorGeom->getDrawFlag(DRAW_SUBPOLY));
	vorviewUI->displaySurfaceCpButton->value(vorGeom->getDrawFlag(DRAW_CP_SURF));
	vorviewUI->displayXSecButton->value(vorGeom->getDrawFlag(DRAW_XSEC));
	vorviewUI->displayCamberButton->value(vorGeom->getDrawFlag(DRAW_CAMBER));

	//==== Slice Flags ====//
	vorviewUI->sliceFlatPlateButton->value(vorGeom->getFlagFlag());
	vorviewUI->sliceSymButton->value(vorGeom->getKeySliceSymFlag() );

	//==== Update Cp Values ====//
	int off = 0;

	if ( vorGeom->getDrawFlag(DRAW_CP_SURF) )
		off = CP_SURF_MIN;

	char str[256];
	sprintf( str, "%6.3f", vorGeom->getCpClamps( CP_MIN+off ));
	vorviewUI->cpMinOutput->value( str );

	sprintf( str, "%6.3f", vorGeom->getCpClamps( CP_MAX+off ));
	vorviewUI->cpMaxOutput->value( str );

	sprintf( str, "%6.3f", 0.5*(vorGeom->getCpClamps( CP_MIN_CLAMP+off ) + vorGeom->getCpClamps( CP_MAX_CLAMP+off )) );
	vorviewUI->cpMidOutput->value( str );

	sprintf( str, "%6.3f", vorGeom->getCpClamps( CP_MIN_CLAMP+off ));
	vorviewUI->cpMinClampOutput->value( str );

	sprintf( str, "%6.3f", vorGeom->getCpClamps( CP_MAX_CLAMP+off ));
	vorviewUI->cpMaxClampOutput->value( str );

	vorviewUI->numSubPanelCounter->value( vorGeom->getNumSubPanels() );

	vorviewUI->enableKeySliceButton->value( vorGeom->getKeySliceTempFlag() );

	if ( vorGeom->getVorSlicePtr()->get_curr_trap() >= 0 )
	{
		int ctrap = vorGeom->getVorSlicePtr()->get_curr_trap();
		vorviewUI->trapCounter->value( ctrap );

		VOROBJ* VorPtr = vorGeom->getVorSlicePtr()->Vor;

		//===== Load Up Control Browser Choice ====//
		vorviewUI->controlBrowser->clear();
		for ( int c = 0 ; c < VorPtr->ntotcsf ; c++ )
		{
			vorviewUI->controlBrowser->add( VorPtr->csfname[c] );
		}
		
		//==== Load Up Curr Control Surface ====//
		int curr_csf = VorPtr->curr_csf;
		if ( curr_csf >= 0 && curr_csf < VorPtr->ntotcsf )
		{
			vorviewUI->controlNameInput->value( VorPtr->csfname[curr_csf] );
			sprintf(str, "%6.2f", VorPtr->csfdefangle[curr_csf] );
			vorviewUI->controlAngInput->value( str );

			sprintf(str, "%6.2f", VorPtr->csfchord[curr_csf] );
			vorviewUI->controlChordInput->value( str );

			sprintf(str, "%6.2f", VorPtr->csfhingepercent[curr_csf] );
			vorviewUI->controlHingeInput->value( str );

			vorviewUI->controlTypeChoice->value( VorPtr->csftype[curr_csf] );
			vorviewUI->controlReflChoice->value( VorPtr->csfreflect[curr_csf] );
			vorviewUI->controlSymChoice->value( VorPtr->csfsymmetry[curr_csf] );

			int r, g, b;
			vorGeom->getVorSlicePtr()->get_csf_color( curr_csf, &r, &g, &b );
			vorviewUI->controlColorBox->color( fl_rgb_color( r, g, b ));
			vorviewUI->controlColorBox->hide();
			vorviewUI->controlColorBox->show();

		}
	    vorviewUI->controlBrowser->value(curr_csf+1);


	}

	vorviewUI->editKeySliceSelector->range( 0, vorGeom->getVorSlicePtr()->get_num_key_slices()-1 );
	vorviewUI->editKeySliceSelector->value( vorGeom->getVorSlicePtr()->get_current_key_slice() );


	m_EditKeySliceYPosSlider->SetVal( vorGeom->getCurrKeySliceYFract() );
	m_EditKeySliceYPosSlider->UpdateGui();

	vorviewUI->numRegionSlices->value( vorGeom->getVorSlicePtr()->get_current_nslices() );


}

void VorviewScreen::hide()
{
	vorGeom->setNoShowFlag(1);
	vorviewUI->UIWindow->hide();
}

void VorviewScreen::position( int x, int y )
{
	vorviewUI->UIWindow->position( x, y );
}

void VorviewScreen::setTitle( const char* name )
{
}

void VorviewScreen::parm_changed( Parm* parm )
{
}

//==== Close Callbacks =====//
void VorviewScreen::closeCB( Fl_Widget* w)
{
	vorGeom->setNoShowFlag(1);
	vorviewUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}
