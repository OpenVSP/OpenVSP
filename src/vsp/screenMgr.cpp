//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// screenMgr.cpp: implementation of the screenMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "screenMgr.h"
#include "geomScreen.h"
#include "structureScreen.h"
#include "cfdMeshScreen.h"
#include "parmScreen.h"
#include "parmLinkScreen.h"
#include "feaStructScreen.h"
#include "groupScreen.h"
#include "wingScreen.h"
#include "fuseScreen.h"
#include "FuselageScreen.h"
#include "havocScreen.h"
#include "extScreen.h"
#include "msWingScreen.h"
#include "hwbScreen.h"
#include "curveEditScreen.h"
#include "blankScreen.h"
#include "meshScreen.h"
#include "hrmScreen.h"
#include "ductScreen.h"
#include "propScreen.h"
#include "engineScreen.h"
#include "labelScreen.h"
#include "vorviewScreen.h"
#include "main.h"
#include "VspPreferences.h"
#include "textureMgrScreen.h"
#include "cabinLayoutScreen.h"
#include "selectFileScreen.h"

#include "scriptMgr.h"

#include "FeaMeshMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScreenMgr::ScreenMgr(Aircraft* airPtr)
{
	antialias = 1;
	aircraftPtr = airPtr;
//	cursorx = cursory = 0;
	createGui();
	mainWinUI->winShell->label(VSPVERSION2);
	mainWinUI->TitleBox->label(VSPVERSION3);

}

ScreenMgr::~ScreenMgr()
{
	delete podScreen;
	delete xsecGeomScreen;
	delete meshScreen;
	delete hrmScreen;
	delete ductScreen;
	delete propScreen;
	delete engineScreen;
	delete blankScreen;
	delete fuseScreen;
	delete fuselageScreen;
	delete havocScreen;
	delete extScreen;
	delete msWingScreen;
	delete hwbScreen;
	delete groupScreen;
	delete cabinLayoutScreen ;

	delete geomScreen;
	delete structureScreen;
	delete cfdMeshScreen;
	delete parmScreen;
	delete parmLinkScreen;
	delete feaStructScreen;
	delete labelScreen;
	delete vorviewScreen;
	delete textureMgrScreen;

	delete selectFileScreen;
	delete curveEditScreen;

	delete glWin; 
	delete scriptOutUI;
	delete scriptErrorBuffer;
	delete scriptUI;
	delete aboutScreen;
	delete viewUI;
	delete aeroRefUI;
	delete massPropUI;
	delete lightingUI;
	delete backgroundUI;
	delete awaveUI;
	delete sliceUI;
	delete compGeomUI;
	delete importFileUI;
	delete exportFileUI;

	delete backgroundWScaleSlider;
	delete backgroundHScaleSlider;
	delete backgroundXOffSlider;
	delete backgroundYOffSlider;
	delete scriptBuffer;

	delete compGeomTextBuffer;
	delete awaveTextBuffer;
	delete aboutScreenTextBuffer;

	delete mainWinUI;

}

void ScreenMgr::createGui()
{
	//===== Create Main Window ====//
	mainWinUI = new MainWinUI();

	mainWinUI->NewMenu->callback( staticMenuCB, this );
	mainWinUI->OpenMenu->callback( staticMenuCB, this );
	mainWinUI->SaveMenu->callback( staticMenuCB, this );
	mainWinUI->SaveAsMenu->callback( staticMenuCB, this );
	mainWinUI->SaveSelMenu->callback( staticMenuCB, this );
	mainWinUI->InsertFileMenu->callback( staticMenuCB, this );
	mainWinUI->ImportMenu->callback( staticMenuCB, this );
	mainWinUI->ExportMenu->callback( staticMenuCB, this );
	mainWinUI->TempDirMenu->callback( staticMenuCB, this );
	mainWinUI->VersionMenu->callback( staticMenuCB, this );
	mainWinUI->ExitMenu->callback( staticMenuCB, this );

	mainWinUI->GeomModifyMenu->callback( staticMenuCB, this );
	mainWinUI->StructureMenu->callback( staticMenuCB, this );
	mainWinUI->CompGeomMenu->callback( staticMenuCB, this );
//	mainWinUI->MeshMenu->callback( staticMenuCB, this );
	mainWinUI->CFDMeshGeomMenu->callback( staticMenuCB, this );
	mainWinUI->ParmLinkMenu->callback( staticMenuCB, this );
	mainWinUI->MassPropMenu->callback( staticMenuCB, this );
	mainWinUI->AeroRefMenu->callback( staticMenuCB, this );
//	mainWinUI->SliceMenu->callback( staticMenuCB, this );
	mainWinUI->AwaveMenu->callback( staticMenuCB, this );
	mainWinUI->OneScreenMenu->callback( staticMenuCB, this );
	mainWinUI->FourScreenMenu->callback( staticMenuCB, this );
	mainWinUI->TwoHorzScreenMenu->callback( staticMenuCB, this );
	mainWinUI->TwoVertScreenMenu->callback( staticMenuCB, this );
	mainWinUI->BackgroundMenu->callback( staticMenuCB, this );
	mainWinUI->LightingMenu->callback( staticMenuCB, this );
	mainWinUI->LabelsMenu->callback( staticMenuCB, this );
	mainWinUI->ScreenShotMenu->callback( staticMenuCB, this );

	mainWinUI->TopViewMenu->callback( staticMenuCB, this );
	mainWinUI->FrontViewMenu->callback( staticMenuCB, this );
	mainWinUI->LeftSideViewMenu->callback( staticMenuCB, this );
	mainWinUI->LeftIsoViewMenu->callback( staticMenuCB, this );
	mainWinUI->BottomViewMenu->callback( staticMenuCB, this );
	mainWinUI->BackViewMenu->callback( staticMenuCB, this );
	mainWinUI->RightSideViewMenu->callback( staticMenuCB, this );
	mainWinUI->RightIsoViewMenu->callback( staticMenuCB, this );
	mainWinUI->CenterViewMenu->callback( staticMenuCB, this );
	mainWinUI->AdjustViewMenu->callback( staticMenuCB, this );
	mainWinUI->AntialiasMenu->callback( staticMenuCB, this );
	mainWinUI->TextureMenu->callback( staticMenuCB, this );

	
	mainWinUI->ScriptMenu->callback( staticMenuCB, this );
	mainWinUI->ScriptOutMenu->callback( staticMenuCB, this );

	mainWinUI->EditVorviewMenu->callback( staticMenuCB, this );
	mainWinUI->RevertMenu->callback( staticMenuCB, this );

	mainWinUI->ShowAbout->callback(staticMenuCB, this);
	mainWinUI->ShowHelpWebsite->callback(staticMenuCB, this);

	//==== Export File Window ====//
	exportFileUI = new ExportFileUI();
	exportFileUI->sterolithButton->callback( staticMenuCB, this );
	exportFileUI->xsecButton->callback( staticMenuCB, this );
	exportFileUI->felisaButton->callback( staticMenuCB, this );
	exportFileUI->rhinoButton->callback( staticMenuCB, this );
	exportFileUI->nascartButton->callback( staticMenuCB, this );
	exportFileUI->cart3dButton->callback( staticMenuCB, this );
	exportFileUI->povrayButton->callback( staticMenuCB, this );
	exportFileUI->gmshButton->callback( staticMenuCB, this );
	exportFileUI->x3dButton->callback( staticMenuCB, this );

	//==== Export File Window ====//
	importFileUI = new ImportFileUI();
	importFileUI->sterolithButton->callback( staticMenuCB, this );
	importFileUI->nascartButton->callback( staticMenuCB, this );
	importFileUI->xsecButton->callback( staticMenuCB, this );
	importFileUI->xsecSurfButton->callback( staticMenuCB, this );

	//==== Comp Geom Screen ====//
	compGeomUI = new CompGeomUI();
	compGeomUI->UIWindow->position( 170, 60 );
	compGeomTextBuffer = new Fl_Text_Buffer();
	compGeomUI->outputTextDisplay->buffer( compGeomTextBuffer );
	compGeomUI->csvFileButton->callback( staticMenuCB, this );
	compGeomUI->csvFileChooseButton->callback( staticMenuCB, this );
	compGeomUI->txtFileChooseButon->callback( staticMenuCB, this );
	compGeomUI->executeButton->callback( staticMenuCB, this );
	compGeomUI->csvFileButton->value(1);
	compGeomUI->tsvFileButton->callback( staticMenuCB, this );
	compGeomUI->tsvFileChooseButton->callback( staticMenuCB, this );
	compGeomUI->tsvFileButton->value(0);

	//==== Slice Screen ====//
	sliceUI = new SliceUI();
	sliceUI->UIWindow->position( 170, 60 );
	sliceUI->startButton->callback( staticMenuCB, this );

	//==== AWave Screen ====//
	awaveUI = new AwaveUI();
	awaveUI->UIWindow->position( 170, 60 );
	awaveTextBuffer = new Fl_Text_Buffer();
	awaveUI->outputTextDisplay->buffer( awaveTextBuffer );
	awaveUI->startButton->callback( staticMenuCB, this );
	awaveUI->planeButton->callback( staticMenuCB, this );
	awaveUI->coneButton->callback( staticMenuCB, this );
	awaveUI->awaveButton->callback( staticMenuCB, this );
	awaveUI->fileButton->callback( staticMenuCB, this );
	awaveUI->planeButton->value(1);
	awaveUI->sliceAngleSlider->deactivate();
	awaveUI->sliceSectionSlider->deactivate();

	//==== Background Screen ====//
	backgroundUI = new BackgroundUI();
	backgroundUI->UIWindow->position( 50, 60 );
	backgroundUI->redSlider->callback( staticMenuCB, this );
	backgroundUI->greenSlider->callback( staticMenuCB, this );
	backgroundUI->blueSlider->callback( staticMenuCB, this );
	backgroundUI->colorBackButton->callback( staticMenuCB, this );
	backgroundUI->jpegBackButton->callback( staticMenuCB, this );

	backgroundUI->preserveAspectButton->callback( staticMenuCB, this );

	backgroundWScaleSlider = new SliderInputCombo( backgroundUI->wScaleSlider, backgroundUI->wScaleInput );
	backgroundWScaleSlider->SetCallback( staticMenuCB, this );
	backgroundWScaleSlider->SetLimits( 0.1, 10.0 );
	backgroundWScaleSlider->SetRange( 0.5 );
	backgroundHScaleSlider = new SliderInputCombo( backgroundUI->hScaleSlider, backgroundUI->hScaleInput );
	backgroundHScaleSlider->SetCallback( staticMenuCB, this );
	backgroundHScaleSlider->SetLimits( 0.1, 10.0 );
	backgroundHScaleSlider->SetRange( 0.5 );

	backgroundXOffSlider = new SliderInputCombo( backgroundUI->xOffsetSlider, backgroundUI->xOffsetInput );
	backgroundXOffSlider->SetCallback( staticMenuCB, this );
	backgroundXOffSlider->SetLimits( -10.0, 10.0 );
	backgroundXOffSlider->SetRange( 0.5 );
	backgroundYOffSlider = new SliderInputCombo( backgroundUI->yOffsetSlider, backgroundUI->yOffsetInput );
	backgroundYOffSlider->SetCallback( staticMenuCB, this );
	backgroundYOffSlider->SetLimits( -10.0, 10.0 );
	backgroundYOffSlider->SetRange( 0.5 );

	backgroundUI->resetDefaultsButton->callback( staticMenuCB, this );

	//==== Lighting Screen ====//
	lightingUI = new LightingUI();
	lightingUI->UIWindow->position( 50, 60 );
	lightingUI->light0Button->callback( staticMenuCB, this );
	lightingUI->light1Button->callback( staticMenuCB, this );
	lightingUI->light2Button->callback( staticMenuCB, this );
	lightingUI->light3Button->callback( staticMenuCB, this );
	lightingUI->light4Button->callback( staticMenuCB, this );
	lightingUI->light5Button->callback( staticMenuCB, this );
	lightingUI->ambSlider->callback( staticMenuCB, this );
	lightingUI->diffSlider->callback( staticMenuCB, this );
	lightingUI->specSlider->callback( staticMenuCB, this );

	//==== Mass Prop Screen ====//
	massPropUI = new MassPropUI();
	massPropUI->UIWindow->position( 550, 250 );
	massPropUI->numSliceSlider->callback( staticMenuCB, this );
	massPropUI->computeButton->callback( staticMenuCB, this );
	massPropUI->drawCgButton->callback( staticMenuCB, this );
	massPropUI->fileExportButton->callback( staticMenuCB, this );

	//==== Aero Ref Screen ====//
	aeroRefUI = new AeroRefUI();
	aeroRefUI->UIWindow->position( 550, 250 );
	aeroRefUI->refCompChoice->callback( staticMenuCB, this );
	aeroRefUI->refCompChoice->textfont( 1 );
	aeroRefUI->refAreaChoice->callback( staticMenuCB, this );
	aeroRefUI->refAreaInput->callback( staticMenuCB, this );
	aeroRefUI->refChordChoice->callback( staticMenuCB, this );
	aeroRefUI->refChordInput->callback( staticMenuCB, this );
	aeroRefUI->refSpanChoice->callback( staticMenuCB, this );
	aeroRefUI->refSpanInput->callback( staticMenuCB, this );
	aeroRefUI->acRefChoice->callback( staticMenuCB, this );
	aeroRefUI->xacInput->callback( staticMenuCB, this );
	aeroRefUI->yacInput->callback( staticMenuCB, this );
	aeroRefUI->zacInput->callback( staticMenuCB, this );
	aeroRefUI->cgRelChoice->callback( staticMenuCB, this );
	aeroRefUI->refXCGInput->callback( staticMenuCB, this );
	aeroRefUI->refYCGInput->callback( staticMenuCB, this );
	aeroRefUI->refZCGInput->callback( staticMenuCB, this );
	aeroRefUI->UIWindow->callback(staticMenuCB, this);
	aeroRefUI->refreshButton->callback(staticMenuCB, this);


	//==== View Screen ====//
	viewUI = new ViewUI();
	viewUI->UIWindow->position( 50, 60 );
	viewUI->xPosPlus1->callback( staticMenuCB, this );
	viewUI->xPosPlus2->callback( staticMenuCB, this );
	viewUI->xPosMinus1->callback( staticMenuCB, this );
	viewUI->xPosMinus2->callback( staticMenuCB, this );
	viewUI->yPosPlus1->callback( staticMenuCB, this );
	viewUI->yPosPlus2->callback( staticMenuCB, this );
	viewUI->yPosMinus1->callback( staticMenuCB, this );
	viewUI->yPosMinus2->callback( staticMenuCB, this );
	viewUI->zoomPlus1->callback( staticMenuCB, this );
	viewUI->zoomPlus2->callback( staticMenuCB, this );
	viewUI->zoomMinus1->callback( staticMenuCB, this );
	viewUI->zoomMinus2->callback( staticMenuCB, this );

	//==== About Screen
	aboutScreen = new AboutScreen();
	aboutScreen->okButton->callback( staticMenuCB, this );

	const char* aboutText1 = "OpenVSP is a parametric aircraft geometry tool.  OpenVSP allows the user to create a 3D model of an aircraft defined by common engineering parameters.  This model can be processed into formats suitable for engineering analysis.\n\n";
        const char* aboutText2 = "The predecessors to OpenVSP have been developed by JR Gloudemans and others for NASA since the early 1990's.  In January 2012, OpenVSP was released as an open source project available to everyone free of charge.\n\n";
        const char* aboutText3 = "\t\twww.openvsp.org\n\n";
        const char* aboutText4 = "OpenVSP is available under the terms of the NASA Open Source Agreement (NOSA) version 1.3.  The complete terms of the license are specified in the LICENSE file which accompanies this program.\n\n";
        const char* aboutText5 = "Copyright (c) 2012 United States Government as represented by the Administrator for The National Aeronautics and Space Administration.  All Rights Reserved.\n";

	aboutScreenTextBuffer = new Fl_Text_Buffer();
	aboutScreen->outputTextDisplay->buffer( aboutScreenTextBuffer );

	aboutScreen->outputTextDisplay->buffer()->insert(0,aboutText1);
	aboutScreen->outputTextDisplay->buffer()->insert(300,aboutText2);
	aboutScreen->outputTextDisplay->buffer()->insert(600,aboutText3);
	aboutScreen->outputTextDisplay->buffer()->insert(900,aboutText4);
	aboutScreen->outputTextDisplay->buffer()->insert(1200,aboutText5);
	aboutScreen->outputTextDisplay->wrap_mode(1, 65);

	//==== Script Screen ====//
	scriptUI = new ScriptUI();
	scriptBuffer = new Fl_Text_Buffer();
    scriptUI->scriptEditor->buffer(scriptBuffer);
	scriptUI->UIWindow->position( 0, 640 );
//	scriptUI->scriptEditor->callback( staticMenuCB, this );
//	scriptUI->scriptEditor->when(FL_WHEN_ENTER_KEY);
	scriptUI->recordButton->callback( staticMenuCB, this );
	scriptUI->executeButton->callback( staticMenuCB, this );
	scriptUI->playButton->callback( staticMenuCB, this );
	scriptUI->clearButton->callback( staticMenuCB, this );
	scriptUI->saveButton->callback( staticMenuCB, this );

	scriptOutUI = new ScriptOutUI();
	scriptErrorBuffer = new Fl_Text_Buffer();
    scriptOutUI->errorDisplay->buffer(scriptErrorBuffer);
	scriptOutUI->UIWindow->position( 0, 860 );
	scriptOutUI->quietButton->callback( staticMenuCB, this );
	scriptOutUI->clearButton->callback( staticMenuCB, this );
	scriptOutUI->closeButton->callback( staticMenuCB, this );


	scriptMgr->init(scriptUI, scriptOutUI);
	scriptMgr->setAircraftPtr( aircraftPtr );
	scriptMgr->setScreenMgr( this );

	/*
	labelUI = new LabelUI();
	labelUI->UIWindow->position( 170, 160 );
	labelUI->xLockButton->callback( staticMenuCB, this );
	labelUI->yLockButton->callback( staticMenuCB, this );
	labelUI->zLockButton->callback( staticMenuCB, this );
*/

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = mainWinUI->GlWinGroup;
	mainWinUI->GlWinGroup->begin();
	glWin = new VspGlWindow(w->x(), w->y(), w->w(), w->h()); 
	mainWinUI->GlWinGroup->end();

	mainWinUI->winShell->callback(staticCloseCB, this);

	glWin->setAircraftPtr( aircraftPtr );

	//==== Create Screens ====//
	podScreen = new PodScreen(this);
	xsecGeomScreen = new XsecGeomScreen(this);
	meshScreen = new MeshScreen(this);
	hrmScreen = new HrmScreen(this);
	ductScreen = new DuctScreen(this);
	propScreen = new PropScreen(this);
	engineScreen = new EngineScreen(this);
	blankScreen = new BlankScreen(this);
//	wingScreen = new WingScreen(this);
	fuseScreen = new FuseScreen(this);
	fuselageScreen = new FuselageScreen(this);
	havocScreen = new HavocScreen(this);
	extScreen = new ExtScreen(this);
	msWingScreen = new MsWingScreen(this);
	hwbScreen = new HwbScreen(this);
	groupScreen = new GroupScreen(this, aircraftPtr);
	cabinLayoutScreen = new CabinLayoutScreen(this);

	geomScreen = new GeomScreen(this, aircraftPtr);
	structureScreen = new StructureScreen(this, aircraftPtr);
	cfdMeshScreen = new CfdMeshScreen( this, aircraftPtr );
	parmScreen = new ParmScreen( this, aircraftPtr );
	parmLinkScreen = new ParmLinkScreen( this, aircraftPtr );
	feaStructScreen = new FeaStructScreen( this, aircraftPtr );
	labelScreen = new LabelScreen(this, aircraftPtr, glWin);
	vorviewScreen = new VorviewScreen(this, aircraftPtr, glWin);
	textureMgrScreen = new TextureMgrScreen( this, aircraftPtr );

	selectFileScreen = new SelectFileScreen();


	curveEditScreen = new CurveEditScreen(this, aircraftPtr);


//	Fl::add_idle((void (*)(void*))App::idleCB, this );
};

void ScreenMgr::closeCB( Fl_Widget* w )
{
	if (Fl::event_state(FL_SHIFT | FL_CTRL)) exit(0);
	switch(fl_choice("VSP is exiting. Save or discard your changes.", "Cancel", "Discard", "Save"))
	{
		case(0):
			return;

		case(1):
			vsp_exit();
//			exit(0);

		case(2):
			s_save(ScriptMgr::GUI);
			vsp_exit();
//			exit(0);
	}
}

void ScreenMgr::hideCB( Fl_Widget* w )
{
	this->hideGui();
}

void ScreenMgr::showGui(int argc, char** argv)
{


	mainWinUI->winShell->show(argc, argv);
	mainWinUI->winShell->position(5,30);

	glWin->init();

	//==== Show Geom Browser Screen ====//
	geomScreen->show();
	geomScreen->position( 610, 30 );

	// Show the About Screen
	//aboutScreen->UIWindow->show();
	//aboutScreen->UIWindow->fullscreen();

	sprintf(labelStr, "File Name: %s", aircraftPtr->getFileName().get_char_star() );
	mainWinUI->FileNameBox->label(labelStr);




//	Fl::flush();		//jrg ???

}

void ScreenMgr::showGui()
{
	mainWinUI->winShell->show();
	mainWinUI->winShell->position(5,30);

	glWin->init();

	//==== Show Geom Browser Screen ====//
	geomScreen->show();
	geomScreen->position( 610, 30 );

	sprintf(labelStr, "File Name: %s", aircraftPtr->getFileName().get_char_star() );
	mainWinUI->FileNameBox->label(labelStr);

   // change the close callback so that it only hides the inteface
   // instead of exiting the application
	mainWinUI->winShell->callback(staticHideCB, this);
 
//   Fl::run();

//	Fl::flush();		//jrg ???

}



void ScreenMgr::hideGui()
{
   mainWinUI->winShell->hide();
   geomScreen->hide();
   this->hideGeomScreens();
}


void ScreenMgr::handleEvents()
{
   Fl::wait();
}


Aircraft* ScreenMgr::getAircraftPtr()
{
   return this->aircraftPtr;
}

//==== Update Subwindows ====//
void ScreenMgr::update( int screenID )
{
	int c;
	char str[256];
	if ( screenID == GEOM_SCREEN )
		geomScreen->update();

	//==== Update Aero Reference Window ====//
	if ( aeroRefUI->UIWindow->shown() )
	{
		//==== Fill GUI ====//
		aeroRefUI->refCompChoice->clear();

		int refind = 0;
		Geom* refGPtr = aircraftPtr->getRefGeom();
		
		for ( c = 0 ; c < (int)aircraftPtr->getGeomVec().size() ; c++ )
		{
			Geom* gPtr = aircraftPtr->getGeomVec()[c];
			aeroRefUI->refCompChoice->add(gPtr->getName());

			if ( gPtr == refGPtr )
				refind = c;
		}
		aeroRefUI->refCompChoice->value(refind);

		if ( refGPtr )
		{
			aeroRefUI->refAreaChoice->value( refGPtr->getRefAreaAutoFlag() );
			sprintf( str, "%6.3f", refGPtr->getRefArea() );
			aeroRefUI->refAreaInput->value( str );

			aeroRefUI->refChordChoice->value( refGPtr->getRefCbarAutoFlag() );
			sprintf( str, "%6.3f", refGPtr->getRefCbar() );
			aeroRefUI->refChordInput->value( str );

			aeroRefUI->refSpanChoice->value( refGPtr->getRefSpanAutoFlag() );
			sprintf( str, "%6.3f", refGPtr->getRefSpan() );
			aeroRefUI->refSpanInput->value( str );

			aeroRefUI->acRefChoice->value( refGPtr->getAeroCenterAutoFlag() );
			vec3d ac = refGPtr->getAeroCenter();
			sprintf( str, "%6.3f", ac.x() );
			aeroRefUI->xacInput->value( str );
			sprintf( str, "%6.3f", ac.y() );
			aeroRefUI->yacInput->value( str );
			sprintf( str, "%6.3f", ac.z() );
			aeroRefUI->zacInput->value( str );

			aeroRefUI->cgRelChoice->value( aircraftPtr->getCGRelAcFlag() );
			vec3d cg = aircraftPtr->getCGLoc();
			if (  aircraftPtr->getCGRelAcFlag() )
				cg = aircraftPtr->getSMLoc();
			sprintf( str, "%6.3f", cg.x() );
			aeroRefUI->refXCGInput->value( str );
			sprintf( str, "%6.3f", cg.y() );
			aeroRefUI->refYCGInput->value( str );
			sprintf( str, "%6.3f", cg.z() );
			aeroRefUI->refZCGInput->value( str );


		
		}
	}
}

void ScreenMgr::hideMeshScreen()
{
	meshScreen->hide();
}

void ScreenMgr::hideGeomScreens()
{
	//==== Hide All Geom Mod Screens ====//
	podScreen->hide();
	xsecGeomScreen->hide();
	meshScreen->hide();
	hrmScreen->hide();
	//xmlScreen->hide();
	ductScreen->hide();
	propScreen->hide();
	engineScreen->hide();
	blankScreen->hide();
	//wingScreen->hide();
	fuseScreen->hide();
	fuselageScreen->hide();
	groupScreen->hide();
	havocScreen->hide();
	extScreen->hide();
	msWingScreen->hide();
	hwbScreen->hide();
	cabinLayoutScreen->hide();
	vorviewScreen->hide();
}

void ScreenMgr::updateGeomScreens() 
{
	vector< Geom* > activeVec = aircraftPtr->getActiveGeomVec();
	
	hideGeomScreens();

	if (activeVec.size() == 1)
	{

		Geom * geomPtr = aircraftPtr->getActiveGeom();
		if (VspPreferences::Instance()->windowMgrStyle == 1)
			getScreen(geomPtr)->position(geomScreen->x() + geomScreen->w() + 8, geomScreen->y());
		else if (VspPreferences::Instance()->windowMgrStyle == 2)
			getScreen(geomPtr)->position(geomScreen->x(), geomScreen->y() + geomScreen->h() + 35);
		getScreen(geomPtr)->show(geomPtr);
	}

	if (activeVec.size() > 1) 
	{	// group modify
		if (VspPreferences::Instance()->windowMgrStyle == 1)
			getGroupScreen()->position(geomScreen->x() + geomScreen->w() + 8, geomScreen->y());
		else if (VspPreferences::Instance()->windowMgrStyle == 2)
			getGroupScreen()->position(geomScreen->x(), geomScreen->y() + geomScreen->h() + 35);
		getGroupScreen()->show( activeVec );
	}
	else
	{
		getGroupScreen()->hide();
	}

	if ( textureMgrScreen->isShown() )	textureMgrScreen->update();

}

void ScreenMgr::updateBackgroundScreen()
{
	if ( glWin->currVWin->getDrawBackImgFlag() )
	{
		backgroundUI->colorBackButton->value(0);
		backgroundUI->jpegBackButton->value(1);
	}
	else
	{
		backgroundUI->colorBackButton->value(1);
		backgroundUI->jpegBackButton->value(0);
	}
	backgroundUI->redSlider->value(glWin->currVWin->getBackR());
	backgroundUI->greenSlider->value(glWin->currVWin->getBackG());
	backgroundUI->blueSlider->value(glWin->currVWin->getBackB());

	backgroundWScaleSlider->SetVal( glWin->currVWin->backImgScaleW );
	backgroundWScaleSlider->UpdateGui();
	backgroundHScaleSlider->SetVal( glWin->currVWin->backImgScaleH );
	backgroundHScaleSlider->UpdateGui();
	backgroundXOffSlider->SetVal( glWin->currVWin->backImgOffsetX );
	backgroundXOffSlider->UpdateGui();
	backgroundYOffSlider->SetVal( glWin->currVWin->backImgOffsetY );
	backgroundYOffSlider->UpdateGui();

	if ( glWin->currVWin->getBackImgPreserveAspect() )
		backgroundUI->preserveAspectButton->value( 1 );
	else
		backgroundUI->preserveAspectButton->value( 0 );
}

VspScreen * ScreenMgr::getScreen(Geom * geomPtr)
{
	switch (geomPtr->getType())
	{
		case POD_GEOM_TYPE:
			return getPodScreen();
		case BLANK_GEOM_TYPE:
			return getBlankScreen();
		//case WING_GEOM_TYPE:
		//	return getWingScreen();	
		case FUSE_GEOM_TYPE:
			return getFuseScreen();
		case HAVOC_GEOM_TYPE:
			return getHavocScreen();
		case EXT_GEOM_TYPE:
			return getExtScreen();
		case MS_WING_GEOM_TYPE:
			return getMsWingScreen();
		case MESH_GEOM_TYPE:
			return getMeshScreen();
		case VOR_GEOM_TYPE:
			return getHrmScreen();
		case DUCT_GEOM_TYPE:
			return getDuctScreen();
		case PROP_GEOM_TYPE:
			return getPropScreen();
		case ENGINE_GEOM_TYPE:
			return getEngineScreen();
		case HWB_GEOM_TYPE:
			return getHwbScreen();
		case FUSELAGE_GEOM_TYPE:
			return getFuselageScreen();
		case CABIN_LAYOUT_GEOM_TYPE:
			return getCabinLayoutScreen();
		case XSEC_GEOM_TYPE:
			return getXsecGeomScreen();

		default:
			return NULL;
	}
}



//==== Main Window Menu CallBacks ====//
void ScreenMgr::menuCB( Fl_Widget* w )
{
	Fl_Menu_* mw = (Fl_Menu_*)w;
	const Fl_Menu_Item* m = mw->mvalue();

	if ( m == mainWinUI->OneScreenMenu )
		glWin->setWindowLayout( VSP_ONE_WIN );

	else if ( m == mainWinUI->FourScreenMenu )
		glWin->setWindowLayout( VSP_FOUR_WIN );

	else if ( m == mainWinUI->TwoHorzScreenMenu )
		glWin->setWindowLayout( VSP_TWO_HORZ_WIN );

	else if ( m == mainWinUI->TwoVertScreenMenu )
		glWin->setWindowLayout( VSP_TWO_VERT_WIN );

	else if ( m == mainWinUI->ScreenShotMenu )
	{
		s_screenshot( ScriptMgr::GUI );
	}

	else if ( m == mainWinUI->BackgroundMenu )
	{	
		updateBackgroundScreen();
		
		if ( glWin->currVWin->getDrawBackImgFlag() )
		{
			backgroundUI->jpegBackButton->value(1);
			backgroundUI->colorBackButton->value(0);
		}
		else
		{
			backgroundUI->jpegBackButton->value(0);
			backgroundUI->colorBackButton->value(1);
		}

		backgroundUI->UIWindow->show();
	}
	else if ( w == backgroundUI->jpegBackButton )
	{
		
		char *newfile = selectFileScreen->FileChooser("Read Background File?", "*.jpg");
//		char *newfile = fl_file_chooser("Read Background File?", "*.jpg", "");
		if ( newfile != NULL )
		{
			glWin->currVWin->setBackImgFile( newfile );
			glWin->currVWin->setDrawBackImgFlag( 1 );
			backgroundUI->colorBackButton->value(0);
		}
		else
		{
			glWin->currVWin->setDrawBackImgFlag( 0 );
			backgroundUI->jpegBackButton->value(0);
			backgroundUI->colorBackButton->value(1);
		}
		glWin->redraw();
	}

	else if ( w == backgroundUI->colorBackButton )
	{
		backgroundUI->jpegBackButton->value(0);
		glWin->currVWin->setDrawBackImgFlag( 0 );
		glWin->redraw();
	}
	else if ( w == backgroundUI->redSlider || w == backgroundUI->greenSlider || w == backgroundUI->blueSlider )
	{
		int r = (int)backgroundUI->redSlider->value();
		int g = (int)backgroundUI->greenSlider->value();
		int b = (int)backgroundUI->blueSlider->value();
		glWin->currVWin->setBackColor( r, g, b );
		glWin->redraw();
	}
	else if ( w == backgroundUI->preserveAspectButton )
	{
		if ( backgroundUI->preserveAspectButton->value() )
			glWin->currVWin->setBackImgPreserveAspect( 1 );
		else
			glWin->currVWin->setBackImgPreserveAspect( 0 );
	}
	else if ( backgroundWScaleSlider->GuiChanged( w ) )
	{
		glWin->currVWin->backImgScaleW = backgroundWScaleSlider->GetVal();
		if ( glWin->currVWin->getBackImgPreserveAspect() )
		{
			glWin->currVWin->backImgScaleH = glWin->currVWin->backImgScaleW; 
			backgroundHScaleSlider->SetVal( glWin->currVWin->backImgScaleH );
			backgroundHScaleSlider->UpdateGui();
		}
		glWin->redraw();
	}
	else if ( backgroundHScaleSlider->GuiChanged( w ) )
	{
		glWin->currVWin->backImgScaleH = backgroundHScaleSlider->GetVal();
		if ( glWin->currVWin->getBackImgPreserveAspect() )
		{
			glWin->currVWin->backImgScaleW = glWin->currVWin->backImgScaleH; 
			backgroundWScaleSlider->SetVal( glWin->currVWin->backImgScaleW );
			backgroundWScaleSlider->UpdateGui();
		}
		glWin->redraw();
	}
	else if ( backgroundXOffSlider->GuiChanged( w ) )
	{
		glWin->currVWin->backImgOffsetX = backgroundXOffSlider->GetVal();
		glWin->redraw();
	}
	else if ( backgroundYOffSlider->GuiChanged( w ) )
	{
		glWin->currVWin->backImgOffsetY = backgroundYOffSlider->GetVal();
		glWin->redraw();
	}
	else if ( w == backgroundUI->resetDefaultsButton )
	{
		glWin->currVWin->resetBackImgDefaults();
		updateBackgroundScreen();
		glWin->redraw();
	}
	else if ( m == mainWinUI->LightingMenu )
	{
		//==== Set Values ====//
		lightingUI->light0Button->value( glWin->getLightFlag( 0 ) );
		lightingUI->light1Button->value( glWin->getLightFlag( 1 ) );
		lightingUI->light2Button->value( glWin->getLightFlag( 2 ) );
		lightingUI->light3Button->value( glWin->getLightFlag( 3 ) );
		lightingUI->light4Button->value( glWin->getLightFlag( 4 ) );
		lightingUI->light5Button->value( glWin->getLightFlag( 5 ) );
		lightingUI->ambSlider->value( glWin->getLightAmb() );
		lightingUI->diffSlider->value( glWin->getLightDiff() );
		lightingUI->specSlider->value( glWin->getLightSpec() );

		lightingUI->UIWindow->show();
	}
	else if ( w == lightingUI->light0Button ||  w == lightingUI->light1Button || 
			  w == lightingUI->light2Button ||  w == lightingUI->light3Button ||
			  w == lightingUI->light4Button ||  w == lightingUI->light5Button )
	{
		glWin->setLightFlag( 0, lightingUI->light0Button->value() );
		glWin->setLightFlag( 1, lightingUI->light1Button->value() );
		glWin->setLightFlag( 2, lightingUI->light2Button->value() );
		glWin->setLightFlag( 3, lightingUI->light3Button->value() );
		glWin->setLightFlag( 4, lightingUI->light4Button->value() );
		glWin->setLightFlag( 5, lightingUI->light5Button->value() );
		glWin->redraw();
	}
	else if ( w == lightingUI->ambSlider || w == lightingUI->diffSlider || w == lightingUI->specSlider )
	{
		glWin->setLightAmb( lightingUI->ambSlider->value() );
		glWin->setLightDiff( lightingUI->diffSlider->value() );
		glWin->setLightSpec( lightingUI->specSlider->value() );
		glWin->redraw();
	}
	else if ( m == mainWinUI->LabelsMenu )
	{
		labelScreen->loadLabelBrowser();
		labelScreen->show();
	}
	else if ( m == mainWinUI->ScriptMenu )
	{
		scriptUI->UIWindow->show();
	} 
	else if ( w == scriptUI->recordButton )
	{
		int val = scriptUI->recordButton->value();
		scriptMgr->setRecord(val);
//		if (val)
//		{
//			scriptMgr->setExecute(0);
//			scriptUI->executeButton->value(0);
//		}
	}
	else if ( w == scriptUI->executeButton )
	{
		int val = scriptUI->executeButton->value();
		scriptMgr->setExecute(val);
//		if (val)
//		{
//			scriptMgr->setRecord(0);
//			scriptUI->recordButton->value(0);
//		}

	}
	else if ( w == scriptUI->playButton )
	{
		scriptMgr->play();
	}
	else if ( w == scriptUI->clearButton )
	{
		scriptMgr->clear();
	}
	else if ( w == scriptUI->saveButton )
	{
		char *newfile = selectFileScreen->FileChooser("Save Script", "*.txt");
//		char *newfile = fl_file_chooser("Save Script", "*.txt", "");
		if ( newfile != NULL )
		{
			scriptMgr->save(newfile);
		}
	}
	else if ( m == mainWinUI->ScriptOutMenu )
	{
		scriptOutUI->UIWindow->show();
	} 
	else if ( w == scriptOutUI->closeButton )
	{
		scriptOutUI->UIWindow->hide();
	}
	else if ( w == scriptOutUI->clearButton )
	{
		scriptMgr->clearErrors();
	}
	else if ( w == scriptOutUI->quietButton )
	{
		scriptMgr->setQuiet(scriptOutUI->quietButton->value());
	}
	else if ( m == mainWinUI->TopViewMenu  )
		glWin->setView( VSP_TOP_VIEW );

	else if ( m == mainWinUI->FrontViewMenu )		// Views
		glWin->setView( VSP_FRONT_VIEW );
	else if ( m == mainWinUI->LeftSideViewMenu  )
		glWin->setView( VSP_LEFT_SIDE_VIEW );
	else if ( m == mainWinUI->LeftIsoViewMenu  )
		glWin->setView( VSP_LEFT_ISO_VIEW );
	else if ( m == mainWinUI->BottomViewMenu  )
		glWin->setView( VSP_BOTTOM_VIEW );
	else if ( m == mainWinUI->BackViewMenu  )
		glWin->setView( VSP_BACK_VIEW );
	else if ( m == mainWinUI->RightSideViewMenu  )
		glWin->setView( VSP_RIGHT_SIDE_VIEW );
	else if ( m == mainWinUI->RightIsoViewMenu  )
		glWin->setView( VSP_RIGHT_ISO_VIEW );

	else if ( m == mainWinUI->CenterViewMenu  )		// Center Aircraft in View
		glWin->setView( VSP_CENTER_VIEW );

	else if ( m == mainWinUI->AdjustViewMenu )		// Adjust View
	{
		viewUI->UIWindow->show();
	}
	else if ( m == mainWinUI->AntialiasMenu )		// Adjust View
	{
		if (antialias)
		{
			mainWinUI->AntialiasMenu->label("Antialias Lines");
			glDisable( GL_LINE_SMOOTH );
			glDisable(GL_POINT_SMOOTH);
			antialias = 0;
		}
		else
		{
			mainWinUI->AntialiasMenu->label("Alias Lines");
			glEnable( GL_LINE_SMOOTH );
			glEnable(GL_POINT_SMOOTH);
			antialias = 1;
		}
	}
	else if ( w == viewUI->xPosMinus1 )				// Adjust X View
	{	glWin->currVWin->currTrack.adj_x( -0.001 );			glWin->redraw();	}	
	else if ( w == viewUI->xPosMinus2 )
	{	glWin->currVWin->currTrack.adj_x( -0.01 );			glWin->redraw();	}	
	else if ( w == viewUI->xPosPlus1 )
	{	glWin->currVWin->currTrack.adj_x( 0.001 );			glWin->redraw();	}	
	else if ( w == viewUI->xPosPlus2 )
	{	glWin->currVWin->currTrack.adj_x( 0.01 );			glWin->redraw();	}	
	else if ( w == viewUI->yPosMinus1 )				// Adjust Y View
	{	glWin->currVWin->currTrack.adj_y( -0.001 );			glWin->redraw();	}	
	else if ( w == viewUI->yPosMinus2 )
	{	glWin->currVWin->currTrack.adj_y( -0.01 );			glWin->redraw();	}	
	else if ( w == viewUI->yPosPlus1 )
	{	glWin->currVWin->currTrack.adj_y( 0.001 );			glWin->redraw();	}	
	else if ( w == viewUI->yPosPlus2 )
	{	glWin->currVWin->currTrack.adj_y( 0.01 );			glWin->redraw();	}	
	else if ( w == viewUI->zoomMinus1 )				// Adjust Zoom
	{	glWin->currVWin->currTrack.adj_scale( -0.00001 );		glWin->redraw();	}	
	else if ( w == viewUI->zoomMinus2 )
	{	glWin->currVWin->currTrack.adj_scale( -0.0001 );		glWin->redraw();	}	
	else if ( w == viewUI->zoomPlus1 )
	{	glWin->currVWin->currTrack.adj_scale( 0.00001 );		glWin->redraw();	}	
	else if ( w == viewUI->zoomPlus2 )
	{	glWin->currVWin->currTrack.adj_scale( 0.0001 );			glWin->redraw();	}	


	else if ( m == mainWinUI->GeomModifyMenu  )		// Show Geom Browser
		geomScreen->show();

	else if ( m == mainWinUI->StructureMenu )
	{
//		feaMeshMgrPtr->Build();
//		mainWinUI->FileNameBox->label("Done");

		feaStructScreen->show();
//		structureScreen->show();
	}

	else if ( m == mainWinUI->CompGeomMenu )		// Comp Geom
	{
		sliceUI->UIWindow->hide();
		s_compgeom(ScriptMgr::GUI);
		scriptMgr->addLine("compgeom");
		/*
		int sliceFlag = 0;
		Geom* geom = aircraftPtr->comp_geom(sliceFlag);
		if ( geom )
		{
			compGeomUI->UIWindow->show();
			compGeomUI->outputTextDisplay->buffer()->loadfile("comp_geom.txt");
			aircraftPtr->setActiveGeom( geom );
			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
		*/
	}
	//else if ( m == mainWinUI->MeshMenu )		// Mesh Geom
	//{
	//	sliceUI->UIWindow->hide();
	//	s_meshgeom(ScriptMgr::GUI);
	//	scriptMgr->addLine("meshgeom");
	//}
	else if ( m == mainWinUI->CFDMeshGeomMenu )		// CFD Mesh Geom
	{
		sliceUI->UIWindow->hide();
		s_cfdmeshgeom(ScriptMgr::GUI);
		scriptMgr->addLine("cfdmeshgeom");
	}
	else if ( m == mainWinUI->ParmLinkMenu )		// Parameter Linking
	{
		s_parmlinkmgr(ScriptMgr::GUI);
		scriptMgr->addLine("parmlink");
	}
	else if ( m == mainWinUI->TextureMenu )			// Texture Mgr Screen
	{
		s_texturemgr(ScriptMgr::GUI);
		scriptMgr->addLine("texturemgr");
	}
	else if ( m == mainWinUI->MassPropMenu )	// Mass Properties Stuff
	{
		Stringc fn = aircraftPtr->getExportFileName( Aircraft::MASS_PROP_TXT_TYPE);
		if ( fn.get_length() > 35 )
		{
			fn.delete_range( 0, fn.get_length()-35 );
			fn.overwrite_at_position(0, "...");
		}
		massPropUI->fileExportOutput->value(fn);

		massPropUI->UIWindow->show();
	}
	else if ( w == massPropUI->computeButton )
	{
		sliceUI->UIWindow->hide();

		int num = (int)massPropUI->numSliceSlider->value();
		s_massprop(ScriptMgr::GUI, num);

		aircraftPtr->setDrawCGFlag( 1 );
		massPropUI->drawCgButton->value(1);
		glWin->redraw();

		//script
		char str[256];
		sprintf(str, "massprop %d", num );
		scriptMgr->addLine(Stringc(str));
	}
	else if ( w == massPropUI->drawCgButton )
	{
		aircraftPtr->setDrawCGFlag( massPropUI->drawCgButton->value() );
		glWin->redraw();
	}
	else if ( w == massPropUI->fileExportButton  )
	{
		char *newfile = FileChooser("Select mass prop output file.", "*.txt");
		if ( newfile != NULL )
		{
			aircraftPtr->setExortFileName( newfile, Aircraft::MASS_PROP_TXT_TYPE );
			Stringc fn = aircraftPtr->getExportFileName( Aircraft::MASS_PROP_TXT_TYPE);
			if ( fn.get_length() > 35 )
			{
				fn.delete_range( 0, fn.get_length()-35 );
				fn.overwrite_at_position(0, "...");
			}
			massPropUI->fileExportOutput->value(fn);
		}
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
	else if ( m == mainWinUI->AeroRefMenu )		// Aerodynamics Reference Stuff
	{
		aeroRefUI->UIWindow->show();
		aircraftPtr->setDrawACFlag(1);
		aircraftPtr->setDrawCGFlag(1);
		update(0);
	}
	else if ( w == aeroRefUI->refCompChoice )		// AeroRef Comp 
	{
		aircraftPtr->setRefGeomIndex( aeroRefUI->refCompChoice->value() );		
		update(0);
	}
	else if ( w == aeroRefUI->refAreaInput )		// AeroRef Area
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
		{
			gPtr->setRefAreaAutoFlag( 0 );
			double val = atof( aeroRefUI->refAreaInput->value() );
			gPtr->setRefArea( val );
		}
		update(0);
		glWin->redraw();
	}
	else if ( w == aeroRefUI->refAreaChoice )		// AeroRef Area
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
			gPtr->setRefAreaAutoFlag( aeroRefUI->refAreaChoice->value() );
		update(0);
	}
	else if ( w == aeroRefUI->refChordInput )		// AeroRef Chord
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
		{
			gPtr->setRefCbarAutoFlag( 0 );
			double val = atof( aeroRefUI->refChordInput->value() );
			gPtr->setRefCbar( val );
		}
		update(0);
		glWin->redraw();
	}
	else if ( w == aeroRefUI->refChordChoice )		// AeroRef Chord
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
			gPtr->setRefCbarAutoFlag( aeroRefUI->refAreaChoice->value() );
		update(0);
	}
	else if ( w == aeroRefUI->refSpanInput )		// AeroRef Span
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
		{
			gPtr->setRefSpanAutoFlag( 0 );
			double val = atof( aeroRefUI->refSpanInput->value() );
			gPtr->setRefSpan( val );
		}
		update(0);
		glWin->redraw();
	}
	else if ( w == aeroRefUI->refSpanChoice )		// AeroRef Span
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
			gPtr->setRefSpanAutoFlag( aeroRefUI->refAreaChoice->value() );
		update(0);
	}
	else if ( w == aeroRefUI->acRefChoice )			// Aero Center
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
			gPtr->setAeroCenterAutoFlag( aeroRefUI->acRefChoice->value() );
		update(0);
	}
	else if ( w == aeroRefUI->xacInput ||  w == aeroRefUI->yacInput || w == aeroRefUI->zacInput)  // Aero Center
	{
		Geom* gPtr = aircraftPtr->getRefGeom();
		if ( gPtr )
		{
			gPtr->setAeroCenterAutoFlag( 0 );
			double xv = atof( aeroRefUI->xacInput->value() );
			double yv = atof( aeroRefUI->yacInput->value() );
			double zv = atof( aeroRefUI->zacInput->value() );
			gPtr->setAeroCenter( vec3d( xv, yv, zv ));
		}
		glWin->redraw();
		update(0);
	}
	else if ( w == aeroRefUI->cgRelChoice )			// CG
	{
		aircraftPtr->setCGRelAcFlag( aeroRefUI->cgRelChoice->value() );
		update(0);
	}
	else if ( w == aeroRefUI->refXCGInput ||  w == aeroRefUI->refYCGInput || w == aeroRefUI->refZCGInput)  // CG
	{
		double xcg = atof( aeroRefUI->refXCGInput->value() );
		double ycg = atof( aeroRefUI->refYCGInput->value() );
		double zcg = atof( aeroRefUI->refZCGInput->value() );

		aircraftPtr->setCGLoc( vec3d( xcg, ycg, zcg ));
		glWin->redraw();
		update(0);
	}
	else if ( w == aeroRefUI->UIWindow )			// Window
	{
		aircraftPtr->setDrawACFlag(0);
		aircraftPtr->setDrawCGFlag(0);
		aeroRefUI->UIWindow->hide();	
		glWin->redraw();
	}
	else if ( w == aeroRefUI->refreshButton )			// Refresh
	{
		glWin->redraw();
		update(0);
	}
//	else if ( m == mainWinUI->SliceMenu )		// Display Slice Screen
//	{
//		sliceUI->UIWindow->show();
//	}
	else if ( w == sliceUI->startButton )		// Start Slicing 
	{

		sliceUI->UIWindow->hide();
		int num = (int)sliceUI->numSliceSlider->value();

		s_oldslice(ScriptMgr::GUI, num);
		scriptMgr->addLineInt("slice", num);
	}
	else if ( m == mainWinUI->AwaveMenu )		// Display Slice Screen
	{
		Stringc fn = aircraftPtr->getExportFileName( Aircraft::SLICE_TXT_TYPE);
		if ( fn.get_length() > 40 )
		{
			fn.delete_range( 0, fn.get_length()-40 );
			fn.overwrite_at_position(0, "...");
		}
		awaveUI->txtFileOutput->value(fn);

		awaveUI->UIWindow->show();

	}
	else if ( w == awaveUI->startButton )		// Start Slicing 
	{
		//awaveUI->UIWindow->hide();
		int style = 0;
		Stringc styleText = "";
		if (awaveUI->planeButton->value())
		{
			style = MeshGeom::SLICE_PLANAR;
			styleText = "plane";
		}
		else if (awaveUI->coneButton->value())
		{
			style = MeshGeom::SLICE_CONIC;
			styleText = "cone";
		}
		else if (awaveUI->awaveButton->value())
		{
			style = MeshGeom::SLICE_AWAVE;
			styleText = "awave";
		}
		int num = (int)awaveUI->numSliceSlider->value();
		double angle = (int)awaveUI->sliceAngleSlider->value();
		int section = (int)awaveUI->sliceSectionSlider->value();
		Stringc filename = aircraftPtr->getExportFileName( Aircraft::SLICE_TXT_TYPE );

		s_awave(ScriptMgr::GUI, style, num, angle, section, filename); 

		//script
		char str[256];
		sprintf(str, "slice %s %d %0.0f %d %s", styleText(), num, angle, section, filename());
		scriptMgr->addLine(Stringc(str));
	}
	else if ( w == awaveUI->planeButton || w == awaveUI->coneButton || w == awaveUI->awaveButton )
	{
		awaveUI->planeButton->value(w == awaveUI->planeButton);
		awaveUI->coneButton->value(w == awaveUI->coneButton);
		awaveUI->awaveButton->value(w == awaveUI->awaveButton);
		
		if (w == awaveUI->planeButton)
		{
			awaveUI->sliceAngleSlider->deactivate();
			awaveUI->sliceSectionSlider->deactivate();
		}
		else
		{
			awaveUI->sliceAngleSlider->activate();
			awaveUI->sliceSectionSlider->activate();
		}
	}
	else if ( w == awaveUI->fileButton )
	{
		char *newfile = FileChooser("Select slice output file.", "*.txt");
		if ( newfile != NULL )
		{
			aircraftPtr->setExortFileName( newfile, Aircraft::SLICE_TXT_TYPE );
			Stringc fn = aircraftPtr->getExportFileName( Aircraft::SLICE_TXT_TYPE);
			if ( fn.get_length() > 40 )
			{
				fn.delete_range( 0, fn.get_length()-40 );
				fn.overwrite_at_position(0, "...");
			}
			awaveUI->txtFileOutput->value(fn);
		}
	}
	else if ( w == compGeomUI->csvFileButton )
	{
		aircraftPtr->setExportCompGeomCsvFile( !!compGeomUI->csvFileButton->value() );
	}
	else if ( w == compGeomUI->csvFileChooseButton )
	{
		char *newfile = FileChooser("Select comp_geom output file.", "*.csv");
		if ( newfile )
		{
			aircraftPtr->setExortFileName( newfile, Aircraft::COMP_GEOM_CSV_TYPE );
			Stringc fn = aircraftPtr->getExportFileName( Aircraft::COMP_GEOM_CSV_TYPE);
			if ( fn.get_length() > 40 )
			{
				fn.delete_range( 0, fn.get_length()-40 );
				fn.overwrite_at_position(0, "...");
			}
			compGeomUI->csvFileOutput->value(fn);
		}
	}
	else if ( w == compGeomUI->tsvFileButton )
	{
		aircraftPtr->setExportDragBuildTsvFile( !!compGeomUI->tsvFileButton->value() );
	}
	else if ( w == compGeomUI->tsvFileChooseButton )
	{
		char *newfile = FileChooser("Select drag build output file.", "*.tsv");
		if ( newfile )
		{
			aircraftPtr->setExortFileName( newfile, Aircraft::DRAG_BUILD_TSV_TYPE  );
			Stringc fn = aircraftPtr->getExportFileName( Aircraft::DRAG_BUILD_TSV_TYPE );
			if ( fn.get_length() > 40 )
			{
				fn.delete_range( 0, fn.get_length()-40 );
				fn.overwrite_at_position(0, "...");
			}
			compGeomUI->tsvFileOutput->value(fn);
		}
	}
	else if ( w == compGeomUI->txtFileChooseButon )
	{
		char *newfile = FileChooser("Select comp_geom output file.", "*.txt");
		if ( newfile )
		{
			aircraftPtr->setExortFileName( newfile, Aircraft::COMP_GEOM_TXT_TYPE );
			Stringc fn = aircraftPtr->getExportFileName( Aircraft::COMP_GEOM_TXT_TYPE);
			if ( fn.get_length() > 40 )
			{
				fn.delete_range( 0, fn.get_length()-40 );
				fn.overwrite_at_position(0, "...");
			}
			compGeomUI->txtFileOutput->value(fn);
		}
	}
	else if ( w == compGeomUI->executeButton )
	{
		int half_flag = compGeomUI->half_mesh_button->value();

		Geom* geom = aircraftPtr->comp_geom(0, 0, half_flag );
		if ( geom )
		{
			aircraftPtr->setActiveGeom( geom );
			Stringc outfn = aircraftPtr->getExportFileName( Aircraft::COMP_GEOM_TXT_TYPE );
			compGeomUI->outputTextDisplay->buffer()->loadfile(outfn);
			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
	}
	else if ( m == mainWinUI->NewMenu  )			// New Geometry
	{
		s_new(ScriptMgr::GUI);
	}
	else if ( m == mainWinUI->OpenMenu  )			// Open Geometry
	{
		s_open(ScriptMgr::GUI, NULL);
	}
	else if ( m == mainWinUI->SaveMenu  )			// Save Geometry
	{
		s_save(ScriptMgr::GUI);
	}
	else if ( m == mainWinUI->SaveAsMenu  )			// Save As Geometry
	{
		s_saveas(ScriptMgr::GUI, NULL);
	}
	else if ( m == mainWinUI->SaveSelMenu  )		// Save Selected Geometry
	{
		s_savesel(ScriptMgr::GUI, NULL);
	}
	else if ( m == mainWinUI->InsertFileMenu )		// Insert Into Current Geometry
	{
		s_insert(ScriptMgr::GUI, NULL);
	}
	else if ( m == mainWinUI->ImportMenu )			// Import Into Current Geometry
	{
		importFileUI->UIWindow->show();
	}
	else if ( m == mainWinUI->EditVorviewMenu )
	{
		if ( aircraftPtr->getGeomVec().size() == 0 )
			fl_alert("Create Some Geometry First");
		else
			vorviewScreen->show(NULL);
	}
	else if ( m == mainWinUI->RevertMenu )
	{
		aircraftPtr->revertEdits();
	}
	else if ( w == 	importFileUI->sterolithButton )	// Import SteroLithography File
	{
		s_import(ScriptMgr::GUI, NULL, Aircraft::STEREOLITH);
	}
	else if ( w == 	importFileUI->nascartButton )	// Import NASCART File
	{
		s_import(ScriptMgr::GUI, NULL, Aircraft::NASCART);
	}
	else if ( w == 	importFileUI->xsecButton )	// Import Cross Section File - To Mesh
	{
		s_import(ScriptMgr::GUI, NULL, Aircraft::XSEC);
	}
	else if ( w == 	importFileUI->xsecSurfButton )	// Import Cross Section File - To Surf
	{
		s_import(ScriptMgr::GUI, NULL, Aircraft::XSECGEOM);
	}
	else if ( m == mainWinUI->ExportMenu )			// Export Current Geometry
	{
		exportFileUI->UIWindow->show();
	}
	else if ( m == mainWinUI->TempDirMenu )			// Set Temp Directory for Intermediate Files
	{
		char *newfile = FileChooser("Select Temp Directory", "*.*");
		if ( newfile != NULL )
			aircraftPtr->setTempDir( newfile, true );

	}
	else if ( w == 	exportFileUI->sterolithButton )	// Export SteroLithography File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::STEREOLITH);
	}
	else if ( w == 	exportFileUI->xsecButton )		// Export CrossSection File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::XSEC);
	}
	else if ( w == 	exportFileUI->felisaButton )	// Export Felisa File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::FELISA);
	}
	else if ( w == 	exportFileUI->rhinoButton )		// Export Rhino File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::RHINO3D);
	}
	else if ( w == 	exportFileUI->nascartButton )		// Export NASCART File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::NASCART);
	}
	else if ( w == 	exportFileUI->cart3dButton )		// Export Cart3d File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::CART3D);
	}
	else if ( w == 	exportFileUI->gmshButton )		// Export GMsh File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::GMSH);
	}
	else if ( w == 	exportFileUI->povrayButton )		// Export POVRAY File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::POVRAY);
	}
	else if ( w == 	exportFileUI->x3dButton )		// Export X3D File
	{
		s_export(ScriptMgr::GUI, NULL, Aircraft::X3D);
	}

	else if ( m == mainWinUI->ShowAbout )
	{
		aboutScreen->UIWindow->show();
	}
	else if ( m == mainWinUI->ShowHelpWebsite )
	{
		#ifdef __APPLE__
			system("open http://vspmanual.webs.com/onlinemanual.htm");
		#elif LINUX
			system("xdg-open http://vspmanual.webs.com/onlinemanual.htm");
		#else
			system("start /max /wait http://vspmanual.webs.com/onlinemanual.htm");
		#endif
	}
	else if ( m == mainWinUI->VersionMenu )
	{
		#ifdef __APPLE__
			system("open http://www.openvsp.org/latest_version.html");
		#elif LINUX
			system("xdg-open http://www.openvsp.org/latest_version.html");
		#else
			system("start /max /wait http://www.openvsp.org/latest_version.html");
		#endif
	}
	else if ( w == aboutScreen->okButton )
	{
		aboutScreen->UIWindow->fullscreen_off(50, 50, 510, 350);
		aboutScreen->UIWindow->hide();
	}

	else if ( m == mainWinUI->ExitMenu )
		closeCB(0);

	

}

void ScreenMgr::s_new(int src) 
{
	aircraftPtr->newFile();
	aircraftPtr->setActiveGeom( 0 );

	if (src == ScriptMgr::GUI || src == ScriptMgr::GUI_SCRIPT)
	{
		hideGeomScreens();
		update( GEOM_SCREEN );
		sprintf(labelStr, "File Name: %s", aircraftPtr->getFileName().get_char_star() );
		mainWinUI->FileNameBox->label(labelStr);
	}

	if (src == ScriptMgr::GUI) scriptMgr->addLine("new");
}

void ScreenMgr::s_open(int src, const char * newfile) 
{
//	Stringc filestring = selectFileScreen->FileChooser( "Open VSP File?", "*.vsp" );

	if (src != ScriptMgr::SCRIPT && newfile == NULL)
		newfile = selectFileScreen->FileChooser( "Open VSP File?", "*.vsp" );
//		newfile = fl_file_chooser("Open VSP File?", "*.vsp", ".", 0);

	if ( newfile != NULL )
	{
		aircraftPtr->openFile(newfile);
		aircraftPtr->setActiveGeom( 0 );
		if (src != ScriptMgr::SCRIPT)
		{
			hideGeomScreens();
			update( GEOM_SCREEN );
		}
		if (src == ScriptMgr::GUI) scriptMgr->addLine("open", newfile);
	}

	if (src != ScriptMgr::SCRIPT)
	{
		sprintf(labelStr, "File Name: %s", aircraftPtr->getFileName().get_char_star() );
		mainWinUI->FileNameBox->label(labelStr);
		mainWinUI->winShell->redraw();
	}


}

void ScreenMgr::s_screenshot(int src) 
{
	char* newfile;
	if ( src != ScriptMgr::SCRIPT )
		newfile = selectFileScreen->FileChooser("Save Screen?", "*.jpg");
//		newfile = fl_file_chooser("Save Screen?", "*.jpg", "");

	if ( newfile != NULL )
	{
		glWin->grabWriteScreen( newfile );
	}
}



void ScreenMgr::s_save(int src)
{
	Stringc fname = aircraftPtr->getFileName(); 
	aircraftPtr->writeFile(fname.get_char_star());
	if (src == ScriptMgr::GUI) scriptMgr->addLine("save");
}

void ScreenMgr::s_saveas(int src, const char * newfile)
{
	if (src == ScriptMgr::SCRIPT)
	{
		aircraftPtr->writeFile(newfile);
	}
	else
	{
		if (newfile == NULL)
			newfile = selectFileScreen->FileChooser("Save VSP File?", "*.vsp");
//			newfile = fl_file_chooser("Save VSP File?", "*.vsp", "");

		if ( newfile != NULL )
		{
			aircraftPtr->writeFile(newfile);
			if (src == ScriptMgr::GUI) scriptMgr->addLine("save", newfile);
		}

		sprintf(labelStr, "File Name: %s", aircraftPtr->getFileName().get_char_star() );
		mainWinUI->FileNameBox->label(labelStr);
		mainWinUI->winShell->redraw();	
	}
}

void ScreenMgr::s_savesel(int src, const char * newfile)
{
	if (src == ScriptMgr::SCRIPT)
	{
		vector< Geom* > gvec = aircraftPtr->getActiveGeomVec(); //geomScreen->getSelectedGeomVec();
		vector< LabelGeom* > lvec =  aircraftPtr->getActiveLabelVec(); //geomScreen->getSelectedLabelVec();
		aircraftPtr->writeFile(newfile, gvec, lvec);
	}
	else
	{
		if (newfile == NULL)
			newfile = selectFileScreen->FileChooser("Save VSP File?", "*.vsp");
//			newfile = fl_file_chooser("Save VSP File?", "*.vsp", "");

		if ( newfile != NULL )
		{
			vector< Geom* > gvec = aircraftPtr->getActiveGeomVec(); //geomScreen->getSelectedGeomVec();
			vector< LabelGeom* > lvec =  aircraftPtr->getActiveLabelVec(); //geomScreen->getSelectedLabelVec();
			aircraftPtr->writeFile(newfile, gvec, lvec);
			if (src == ScriptMgr::GUI) scriptMgr->addLine("save -sel", newfile);
		}
	}
}


void ScreenMgr::s_insert(int src, const char * newfile)
{
	if (src != ScriptMgr::SCRIPT && newfile == NULL)
		newfile = selectFileScreen->FileChooser("Insert VSP File?", "*.vsp");
//		newfile = fl_file_chooser("Insert VSP File?", "*.vsp", "");
	

	if ( newfile != NULL )
	{
		aircraftPtr->insertFile(newfile);
		aircraftPtr->setActiveGeom( 0 );
		if (src != ScriptMgr::SCRIPT)
		{
			geomScreen->deselectGeomBrowser();
			hideGeomScreens();
			update( GEOM_SCREEN );
		}
		if (src == ScriptMgr::GUI) scriptMgr->addLine("insert", newfile);
	}
}

void ScreenMgr::s_import(int src, const char * newfile, int type)
{
	if (type == -1) // special case to popup import screen
	{
		importFileUI->UIWindow->show();
		return;
	}
	if (src != ScriptMgr::SCRIPT && newfile == NULL)
	{
		if (type == Aircraft::STEREOLITH)
			newfile = selectFileScreen->FileChooser("Read STL File?", "*.stl");
		else if (type == Aircraft::NASCART)
			newfile = selectFileScreen->FileChooser("Read NASCART File?", "bodyin.dat");
		else if (type == Aircraft::XSEC)
			newfile = selectFileScreen->FileChooser("Read Cross Sect File?", "*.hrm");
		else if (type == Aircraft::XSECGEOM)
			newfile = selectFileScreen->FileChooser("Read Cross Sect File?", "*.hrm");
	}

	if ( newfile  != NULL )
	{
		Geom* geom = NULL;
		if (type == Aircraft::STEREOLITH)
		{
			geom = aircraftPtr->read_stl_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("import -stereo", newfile);
		}
		else if (type == Aircraft::NASCART) 
		{
			geom = aircraftPtr->read_nascart_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("import -nascart", newfile);
		}
		else if (type == Aircraft::VORXSEC) 
		{
			geom = aircraftPtr->read_vorxsec_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("import -vorxsec", newfile);
		}
		else if (type == Aircraft::XSEC) 
		{
			geom = aircraftPtr->read_xsec_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("import -xsec", newfile);
		}
		else if (type == Aircraft::XSECGEOM ) 
		{
			geom = aircraftPtr->read_xsecgeom_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("import -xsecgeom", newfile);
		}
		if ( geom )
		{
			aircraftPtr->setActiveGeom( geom );
			hideGeomScreens();
			if (src != ScriptMgr::SCRIPT)
			{
				geomScreen->deselectGeomBrowser();
				update( GEOM_SCREEN );
				geomScreen->selectGeomBrowser(geom);
			}
		}
	}
	if (src != ScriptMgr::SCRIPT)
		importFileUI->UIWindow->hide();

}

void ScreenMgr::s_export(int src, const char * newfile, int type)
{
	//==== Check for Valid STL Mesh =====//
	//if ( type == Aircraft::STEREOLITH )
	//{
	//	if ( !aircraftPtr->check_for_stl_mesh() )
	//	{
	//		fl_alert("No mesh present - run comp_geom to generate stl mesh");
	//		return;
	//	}
	//}

	if (src != ScriptMgr::SCRIPT && newfile == NULL)
	{
		if (type == Aircraft::FELISA)
			newfile = selectFileScreen->FileChooser("Write Felisa File?", "*.fel");
		else if (type == Aircraft::XSEC)
			newfile = selectFileScreen->FileChooser("Write XSec File?", "*.hrm");
		else if (type == Aircraft::STEREOLITH)
			newfile = selectFileScreen->FileChooser("Write STL File?", "*.stl");
		else if (type == Aircraft::RHINO3D)
			newfile = selectFileScreen->FileChooser("Write Rhino3D File?", "*.3dm");
		else if (type == Aircraft::NASCART)
			newfile = selectFileScreen->FileChooser("Write NASCART Files?", "*.dat");
		else if (type == Aircraft::CART3D )
			newfile = selectFileScreen->FileChooser("Write Cart3D Files?", "*.tri");
		else if (type == Aircraft::GMSH )
			newfile = selectFileScreen->FileChooser("Write GMsh Files?", "*.msh");
		else if (type == Aircraft::POVRAY)
			newfile = selectFileScreen->FileChooser("Write POVRAY File?", "*.pov");
		else if (type == Aircraft::X3D)
			newfile = selectFileScreen->FileChooser("Write X3D File?", "*.x3d");
		else if (type == -1)
		{
			exportFileUI->UIWindow->show();
			return;
		}
	}

	if ( newfile != NULL )
	{
		if (type == Aircraft::FELISA)
		{
			Stringc base;
			Stringc fname = newfile;
			int dot_loc = fname.search_for_substring(".");
			base = fname.get_range(0, dot_loc-1);

			Stringc background_file = fname.get_range(0, dot_loc-1);
			background_file.concatenate(".bac");
			Stringc fps3d_bco_file  = fname.get_range(0, dot_loc-1);
			fps3d_bco_file.concatenate(".bsf");

			aircraftPtr->write_felisa_file( newfile );
			aircraftPtr->write_felisa_background_file( (char*)background_file );
			aircraftPtr->write_fps3d_bco_file( (char*)fps3d_bco_file);
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -felisa", newfile);
		}
		else if (type == Aircraft::XSEC)
		{
			aircraftPtr->write_xsec_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -xsec", newfile);
		}
		else if (type == Aircraft::STEREOLITH)
		{
			aircraftPtr->write_stl_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -stereo", newfile);
		}
		else if (type == Aircraft::RHINO3D)
		{
			aircraftPtr->write_rhino_file( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -rhino", newfile);
		}
		else if (type == Aircraft::NASCART)
		{
			aircraftPtr->write_nascart_files( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -nascart", newfile);
		}
		else if (type == Aircraft::CART3D)
		{
			aircraftPtr->write_cart3d_files( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -cart3d", newfile);
		}
		else if (type == Aircraft::GMSH )
		{
			aircraftPtr->write_gmsh_files( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -gmsh", newfile);
		}
		else if (type == Aircraft::POVRAY )
		{
			aircraftPtr->write_povray_file ( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -povray", newfile);
		}
		else if (type == Aircraft::X3D )
		{
			aircraftPtr->write_x3d_file ( newfile );
			if (src == ScriptMgr::GUI) scriptMgr->addLine("export -x3d", newfile);
		}
	}
	if (src != ScriptMgr::SCRIPT)
		exportFileUI->UIWindow->hide();

}


void ScreenMgr::s_compgeom(int src)
{
	if (src != ScriptMgr::SCRIPT)
	{
		compGeomUI->outputTextDisplay->buffer()->text("");			// Clear results
		Stringc fn = aircraftPtr->getExportFileName( Aircraft::COMP_GEOM_TXT_TYPE);
		if ( fn.get_length() > 40 )
		{
			fn.delete_range( 0, fn.get_length()-40 );
			fn.overwrite_at_position(0, "...");
		}
		compGeomUI->txtFileOutput->value(fn);

		Stringc fncvs = aircraftPtr->getExportFileName( Aircraft::COMP_GEOM_CSV_TYPE);
		if ( fncvs.get_length() > 40 )
		{
			fncvs.delete_range( 0, fncvs.get_length()-40 );
			fncvs.overwrite_at_position(0, "...");
		}
		compGeomUI->csvFileOutput->value(fncvs);

		Stringc fntsv = aircraftPtr->getExportFileName( Aircraft::DRAG_BUILD_TSV_TYPE );
		if ( fntsv.get_length() > 40 )
		{
			fntsv.delete_range( 0, fncvs.get_length()-40 );
			fntsv.overwrite_at_position(0, "...");
		}
		compGeomUI->tsvFileOutput->value(fntsv);

		compGeomUI->UIWindow->show();


	}
	else
	{
		Geom* geom = aircraftPtr->comp_geom(0);
	}

//	Geom* geom = aircraftPtr->comp_geom(0);
//	if ( geom )
//	{
//		aircraftPtr->setActiveGeom( geom );
//		if (src != ScriptMgr::SCRIPT)
//		{
//			compGeomUI->UIWindow->show();
////			compGeomUI->resultsLabel->label("Results also written to \"comp_geom.txt\"");
//			compGeomUI->outputTextDisplay->buffer()->loadfile("comp_geom.txt");
//			geomScreen->deselectGeomBrowser();
//			update( GEOM_SCREEN );
//			geomScreen->selectGeomBrowser(geom);
//		}
//	}
}

void ScreenMgr::s_meshgeom(int src)
{
	Geom* geom = aircraftPtr->comp_geom(0, 1);	//Slice 0  Mesh 1
	if ( geom )
	{
		aircraftPtr->setActiveGeom( geom );
		if (src != ScriptMgr::SCRIPT)
		{
			compGeomUI->UIWindow->show();
//			compGeomUI->resultsLabel->label("Results also written to \"comp_geom.txt\"");
			compGeomUI->outputTextDisplay->buffer()->loadfile("comp_geom.txt");
			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
	}
}

void ScreenMgr::s_cfdmeshgeom(int src)
{
	if (src != ScriptMgr::SCRIPT)
	{
		cfdMeshScreen->show();
	}
}

void ScreenMgr::s_parmlinkmgr(int src)
{
	if (src != ScriptMgr::SCRIPT)
	{
		parmLinkScreen->show();
	}
}

void ScreenMgr::s_texturemgr(int src)
{
	if (src != ScriptMgr::SCRIPT)
	{
		textureMgrScreen->show();
	}
}


void ScreenMgr::s_massprop(int src, int num)
{
	Geom* geom = aircraftPtr->massprop(num);
	if ( geom )
	{
		aircraftPtr->setActiveGeom( geom );
		if (src != ScriptMgr::SCRIPT)
		{
			char str[255];
			sprintf(str, "  %6.3f", aircraftPtr->get_total_mass() );	
			massPropUI->totalMassOutput->value( str );

			vec3d cg = aircraftPtr->getCGLoc();
			sprintf(str, "  %6.3f", cg.x() );	massPropUI->xCgOuput->value( str );
			sprintf(str, "  %6.3f", cg.y() );	massPropUI->yCgOuput->value( str );
			sprintf(str, "  %6.3f", cg.z() );	massPropUI->zCgOuput->value( str );

			vec3d momi = aircraftPtr->get_Ixx_Iyy_Ixx();
			sprintf(str, "  %6.3f", momi.x() );	massPropUI->ixxOuput->value( str );
			sprintf(str, "  %6.3f", momi.y() );	massPropUI->iyyOutput->value( str );
			sprintf(str, "  %6.3f", momi.z() );	massPropUI->izzOutput->value( str );

			vec3d prodi = aircraftPtr->get_Ixy_Ixz_Iyz();
			sprintf(str, "  %6.3f", prodi.x() );	massPropUI->ixyOutput->value( str );
			sprintf(str, "  %6.3f", prodi.y() );	massPropUI->ixzOutput->value( str );
			sprintf(str, "  %6.3f", prodi.z() );	massPropUI->iyzOutput->value( str );

			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
	}
}

void ScreenMgr::s_oldslice(int src, int num)
{
	Geom* geom = aircraftPtr->comp_geom(num);
	if ( geom )
	{
		aircraftPtr->setActiveGeom( geom );
		if (src != ScriptMgr::SCRIPT)
		{
			compGeomUI->UIWindow->show();
//			compGeomUI->resultsLabel->label("Results also written to \"compgeom.txt\"");
			compGeomUI->outputTextDisplay->buffer()->loadfile("comp_geom.txt");
			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
	}
}


void ScreenMgr::s_awave(int src, int style, int num, double angle, int section, Stringc filename)
{
	Geom* geom = aircraftPtr->slice(style, num, angle, section, filename);
	static char lstr[512];
	if ( geom )
	{
		aircraftPtr->setActiveGeom( geom );
		if (src != ScriptMgr::SCRIPT)
		{
//			compGeomUI->UIWindow->show();
//			sprintf(lstr, "Results also written to \"%s\"", filename.get_char_star());
//			compGeomUI->resultsLabel->label(lstr);
			awaveUI->outputTextDisplay->buffer()->loadfile(filename.get_char_star());
			geomScreen->deselectGeomBrowser();
			update( GEOM_SCREEN );
			geomScreen->selectGeomBrowser(geom);
		}
	}
}

void ScreenMgr::showParmScreen(Parm* p, int x, int y)
{
	parmScreen->position( x, y );
	parmScreen->show(p);
}

char* ScreenMgr::FileChooser( const char* title, const char* filter )
{ 
	return selectFileScreen->FileChooser( title, filter ); 
}

void ScreenMgr::MessageBox( const char* msg )
{
	fl_message( msg );
}

