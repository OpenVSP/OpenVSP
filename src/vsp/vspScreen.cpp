//****************************************************************************
//    
//   VspScreen Base Class
//   
//   J.R. Gloudemans - 4/21/03
// 
//****************************************************************************

#include "vspScreen.h"
#include "screenMgr.h"
#include "materialMgr.h"
#include "VspPreferences.h"
#include "scriptMgr.h"

#include <FL/Fl_File_Chooser.H>

//==== Constructor =====//
VspScreen::VspScreen(ScreenMgr* mgr)
{
	screenMgrPtr = mgr;
}

//==== Destructor =====//
VspScreen::~VspScreen()
{
}

//****************************************************************************
//****************************************************************************
//****************************************************************************

//==== Constructor =====//
CompScreen::CompScreen(ScreenMgr* mgr) : VspScreen(mgr)
{
	screenMgrPtr = mgr;

	//==== Init Pointers ====//
	geomPtr  = 0;
	initFlag = 0;

	colorButton = primColorButton = 0;
	relButton = absButton = 0;
	noneSymMenu = xySymMenu = yzSymMenu = xzSymMenu = 0;
	nameInput = 0;

	attachFixedButton = attachUVButton = attachMatrixButton = 0;
	acceptScaleButton = resetScaleButton = 0;

	redSlider = greenSlider = blueSlider = 0;
	redButton = greenButton = blueButton = yellowButton = cyanButton = 0;
	magentaButton = darkBlueButton = blackButton = greyButton = 0;

	materialChoice = symmetryChoice = 0;

	xLocSlider = yLocSlider = zLocSlider = 0;
	xLocInput  = yLocInput  = zLocInput = 0;


	xRotSlider = yRotSlider = zRotSlider = 0;
	xRotInput  = yRotInput  = zRotInput = 0;

	rotOriginSlider = 0;
	rotOriginInput = 0;

	uSlider = vSlider = 0;
	uInput  = vInput  = 0;

	numXsecsSlider = numPntsSlider = 0;
	numXsecsInput  = numPntsInput  = 0;

	scaleSlider = 0;
	scaleInput = 0;

	outputNameChoice = 0;
	outputEnableButton = 0;

	densityInput = 0;
	shellMassAreaInput = 0;
	shellEnableButton = 0;
	priorityCounter = 0;

	xLocButton = 0;
	yLocButton = 0;
	zLocButton = 0;
	xRotButton = 0;
	yRotButton = 0;
	zRotButton = 0;
	originButton = 0;
	numPntsButton = 0;
	numXsecsButton = 0;
	densityButton = 0;
	shellMassAreaButton = 0;

}

//==== Destructor =====//
CompScreen::~CompScreen()
{
}

//==== Init =====//
void CompScreen::init()
{
	int i;

	//==== Crappy Check to Make Sure Everything is Defined ====//
	int okFlag = 0;
	if ( colorButton && primColorButton && relButton && absButton && 
		noneSymMenu && xySymMenu && yzSymMenu && xzSymMenu && nameInput &&
		attachFixedButton && attachUVButton && attachMatrixButton &&
		acceptScaleButton && resetScaleButton &&
		redSlider && greenSlider && blueSlider &&
		redButton && greenButton && blueButton && yellowButton && cyanButton &&
		magentaButton && darkBlueButton && blackButton && greyButton &&
		materialChoice && symmetryChoice &&
		xLocSlider && yLocSlider && zLocSlider &&
		xLocInput  && yLocInput  && zLocInput &&
		xRotSlider && yRotSlider && zRotSlider &&
		xRotInput  && yRotInput  && zRotInput &&
		rotOriginSlider && rotOriginInput &&
		uSlider && vSlider && uInput  && vInput  &&
		numXsecsSlider && numPntsSlider &&
		numXsecsInput  && numPntsInput  &&
		scaleSlider && scaleInput && 
		outputNameChoice && outputEnableButton 
		 )
	{
		okFlag = 1;
	}

	if ( !okFlag )
		printf( "CompScreen::init ERROR = Undefined GUI Ptrs \n" );

	relButton->callback( staticScreenCB, this );
	absButton->callback( staticScreenCB, this );

	noneSymMenu->callback( staticScreenCB, this );
	xySymMenu->callback( staticScreenCB, this );
	yzSymMenu->callback( staticScreenCB, this );
	xzSymMenu->callback( staticScreenCB, this );

	nameInput->callback( staticScreenCB, this );

	attachFixedButton->callback( staticScreenCB, this );
	attachUVButton->callback( staticScreenCB, this );
	attachMatrixButton->callback( staticScreenCB, this );

	acceptScaleButton->callback( staticScreenCB, this );
	resetScaleButton->callback( staticScreenCB, this );

	redSlider->callback( staticScreenCB, this );
	redSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	greenSlider->callback( staticScreenCB, this );
	greenSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	blueSlider->callback( staticScreenCB, this );
	blueSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

	redButton->callback( staticScreenCB, this );
	greenButton->callback( staticScreenCB, this );
	blueButton->callback( staticScreenCB, this );
	yellowButton->callback( staticScreenCB, this );
	cyanButton->callback( staticScreenCB, this );
	magentaButton->callback( staticScreenCB, this );
	darkBlueButton->callback( staticScreenCB, this ); 
	blackButton->callback( staticScreenCB, this ); 
	greyButton->callback( staticScreenCB, this );

	materialChoice->clear();
	int num = matMgrPtr->getNumMaterial();
	for ( i = 0 ; i < num ; i++ )
	{
		Material* mat = matMgrPtr->getMaterial(i);
		if ( mat )
			materialChoice->add( mat->name );
	}
	materialChoice->callback( staticScreenCB, this );

	outputEnableButton->callback( staticScreenCB, this );
	outputNameChoice->clear();	
	num = VspPreferences::Instance()->getNumOutputNames();
	for ( i = 0 ; i < num ; i++ )
	{
		outputNameChoice->add( VspPreferences::Instance()->getOutputName(i) );
	}
	outputNameChoice->callback( staticScreenCB, this );


	// script output
	uSlider->setScriptText("gen setu", 0);
	vSlider->setScriptText("gen setv", 0);
	numXsecsSlider->setScriptText("gen tessxsec", 1);
	numPntsSlider->setScriptText("gen tesspoints", 1);

	xLocSlider->setScriptText("xform xloc", 0);
	yLocSlider->setScriptText("xform yloc", 0);
	zLocSlider->setScriptText("xform zloc", 0);

	xRotSlider->setScriptText("xform xrot", 0);
	yRotSlider->setScriptText("xform yrot", 0);
	zRotSlider->setScriptText("xform zrot", 0);

	rotOriginSlider->setScriptText("xform rotorig", 0);
//	scaleSlider->setScriptText("xform scale", 0);

	//jrg mass
	if ( shellEnableButton )
		shellEnableButton->callback( staticScreenCB, this );
	if ( priorityCounter )
		priorityCounter->callback( staticScreenCB, this );

}


//==== Show =====//
void CompScreen::show(Geom* currGeom)
{
	geomPtr = currGeom;

	if ( currGeom->getRelXFormFlag() )
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

		relButton->value(1);
		absButton->value(0);

		if ( xLocButton ) xLocButton->set_parm_ptr( &currGeom->xRelLoc );
		if ( yLocButton ) yLocButton->set_parm_ptr( &currGeom->yRelLoc );
		if ( zLocButton ) zLocButton->set_parm_ptr( &currGeom->zRelLoc );
		if ( xRotButton ) xRotButton->set_parm_ptr( &currGeom->xRelRot );
		if ( yRotButton ) yRotButton->set_parm_ptr( &currGeom->yRelRot );
		if ( zRotButton ) zRotButton->set_parm_ptr( &currGeom->zRelRot );
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

		relButton->value(0);
		absButton->value(1);

		if ( xLocButton ) xLocButton->set_parm_ptr( &currGeom->xLoc );
		if ( yLocButton ) yLocButton->set_parm_ptr( &currGeom->yLoc );
		if ( zLocButton ) zLocButton->set_parm_ptr( &currGeom->zLoc );
		if ( xRotButton ) xRotButton->set_parm_ptr( &currGeom->xRot );
		if ( yRotButton ) yRotButton->set_parm_ptr( &currGeom->yRot );
		if ( zRotButton ) zRotButton->set_parm_ptr( &currGeom->zRot );
	}

	rotOriginSlider->set_parm_ptr( &currGeom->origin );
	rotOriginInput->set_parm_ptr( &currGeom->origin );

	uSlider->set_parm_ptr( &currGeom->uAttach );
	vSlider->set_parm_ptr( &currGeom->vAttach );
	uInput->set_parm_ptr( &currGeom->uAttach );
	vInput->set_parm_ptr( &currGeom->vAttach );

	scaleSlider->set_parm_ptr( &currGeom->scaleFactor  );
	scaleInput->set_parm_ptr( &currGeom->scaleFactor );

	numXsecsSlider->set_parm_ptr( &currGeom->numXsecs );
	numXsecsInput->set_parm_ptr( &currGeom->numXsecs );
	numPntsSlider->set_parm_ptr( &currGeom->numPnts );
	numPntsInput->set_parm_ptr( &currGeom->numPnts );

	nameInput->value( currGeom->getName().get_char_star() );

	symmetryChoice->value( currGeom->getSymCode() );
	materialChoice->value( currGeom->getMaterialID() );

	vec3d c = currGeom->getColor();
	primColorButton->color( fl_rgb_color((int)c.x(), (int)c.y(), (int)c.z()) );

	redSlider->value( c.x() );
	greenSlider->value( c.y() );
	blueSlider->value( c.z() );

	attachFixedButton->value(0);
	attachUVButton->value(0);
	attachMatrixButton->value(0);

	if ( currGeom->getPosAttachFlag() == POS_ATTACH_FIXED )
		attachFixedButton->value(1);
	else if ( currGeom->getPosAttachFlag() == POS_ATTACH_UV )
		attachUVButton->value(1);
	else if ( currGeom->getPosAttachFlag() == POS_ATTACH_MATRIX )
		attachMatrixButton->value(1);

	outputEnableButton->value(currGeom->getOutputFlag());
	outputNameChoice->value(currGeom->getOutputNameID() );

	//jrg mass
	if ( densityInput )
		densityInput->set_parm_ptr( &currGeom->density );
	if ( shellMassAreaInput )
		shellMassAreaInput->set_parm_ptr( &currGeom->shellMassArea );

	if ( shellEnableButton )
		shellEnableButton->value(currGeom->getShellFlag());
	if ( priorityCounter )
		priorityCounter->value(currGeom->getMassPrior());

	if ( originButton ) originButton->set_parm_ptr( &currGeom->origin );
	if ( numPntsButton ) numPntsButton->set_parm_ptr( &currGeom->numPnts );
	if ( numXsecsButton ) numXsecsButton->set_parm_ptr( &currGeom->numXsecs );
	if ( densityButton ) densityButton->set_parm_ptr( &currGeom->density  );
	if ( shellMassAreaButton ) shellMassAreaButton->set_parm_ptr( &currGeom->shellMassArea  );
	

}

void CompScreen::showTextureInfo(Geom* currGeom)
{

}


//==== CallBacks =====//
void CompScreen::screenCB( Fl_Widget* w)
{
	if ( !geomPtr )
		return;

	AppliedTex* currTex = geomPtr->getTex( geomPtr->getCurrentTexID() );

	if ( w == absButton  )
	{
		relButton->value(0);
		absButton->value(1);
		geomPtr->setRelXFormFlag( 0 );
		show( geomPtr );
		xLocInput->update();
		xLocSlider->update();
		scriptMgr->addLine("xform", "position", "abs");
	}
	else if ( w == relButton )
	{
		relButton->value(1);
		absButton->value(0);
		geomPtr->setRelXFormFlag( 1 );
		show( geomPtr );
		xLocInput->update();
		xLocSlider->update();
		scriptMgr->addLine("xform", "position", "rel");
	}
	else if ( w == symmetryChoice )
	{
		Fl_Menu_* mw = (Fl_Menu_*)w;
		const Fl_Menu_Item* m = mw->mvalue();

		if ( m == noneSymMenu )
		{
			geomPtr->setSymCode( NO_SYM );
			scriptMgr->addLine("xform", "symmetry", "none");
		}
		else if (  m == xySymMenu )
		{
			geomPtr->setSymCode( XY_SYM );
			scriptMgr->addLine("xform", "symmetry", "xy");
		}
		else if (  m == xzSymMenu )
		{
			geomPtr->setSymCode( XZ_SYM );
			scriptMgr->addLine("xform", "symmetry", "xz");
		}
		else if (  m == yzSymMenu )
		{
			geomPtr->setSymCode( YZ_SYM );
			scriptMgr->addLine("xform", "symmetry", "yz");
		}
	}
	else if ( w == nameInput )
	{
		Stringc name = nameInput->value();
		geomPtr->setName( name );

		//==== Show as Window Label ====//
		setTitle( name.get_char_star() );
		screenMgrPtr->update( GEOM_SCREEN );

		name.quote('\"');
		scriptMgr->addLine("gen", "name", name);
	}
	else if ( w == attachFixedButton  )
	{
		if ( attachFixedButton->value() )
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_FIXED );
			scriptMgr->addLine("gen", "attach", "fixed");
		}
		else
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_NONE );
			scriptMgr->addLine("gen", "attach", "none");
		}
		attachUVButton->value(0);
		attachMatrixButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );

	}
	else if ( w == attachUVButton )
	{
		if ( attachUVButton->value() )
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_UV );
			scriptMgr->addLine("gen", "attach", "uv");
		}
		else
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_NONE );
			scriptMgr->addLine("gen", "attach", "none");
		}
		attachFixedButton->value(0);
		attachMatrixButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );
	}
	else if ( w == attachMatrixButton )
	{
		if ( attachMatrixButton->value() )
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_MATRIX );
			scriptMgr->addLine("gen", "attach", "matrix");
		}
		else
		{
			geomPtr->setPosAttachFlag( POS_ATTACH_NONE );
			scriptMgr->addLine("gen", "attach", "none");
		}
		attachFixedButton->value(0);
		attachUVButton->value(0);
		screenMgrPtr->update( GEOM_SCREEN );
	}	
	else if ( w == acceptScaleButton )
	{
		scriptMgr->addLineDouble("xform scale", geomPtr->scaleFactor());
		geomPtr->acceptScaleFactor();
	}
	else if ( w == resetScaleButton )
	{
		geomPtr->resetScaleFactor();
	}
	else if ( w == redSlider || w == greenSlider	|| w == blueSlider )				
	{

		int r = (int)redSlider->value();
		int g = (int)greenSlider->value();
		int b = (int)blueSlider->value();

		geomPtr->setColor( r, g, b );

		primColorButton->color( fl_rgb_color(r, g, b) );

		primColorButton->redraw();

		if (!Fl::event_state(FL_BUTTONS))
			scriptMgr->addLine("gen", "color", r, g, b);
	}
	else if ( w == redButton )
		setColor( 255, 0, 0 );
	else if ( w == greenButton )
		setColor( 0, 255, 0 );
	else if ( w == blueButton )
		setColor( 0, 0, 255 );
	else if ( w == yellowButton )
		setColor( 255, 255, 0 );
	else if ( w == cyanButton )
		setColor( 0, 255, 255 );
	else if ( w == magentaButton )
		setColor( 255, 0, 255 );
	else if ( w == darkBlueButton )
		setColor( 0, 0, 123 );
	else if ( w == blackButton )
		setColor( 0, 0, 0 );
	else if ( w == greyButton )
		setColor( 123, 123, 123 );

	else if ( w == materialChoice )
	{
		geomPtr->setMaterialID( materialChoice->value() );
		Stringc matName = matMgrPtr->getMaterial(materialChoice->value())->name();
		matName.quote('\"');
//		char str[128];
//		sprintf(str, "\"%s\"", matMgrPtr->getMaterial(materialChoice->value())->name());
		scriptMgr->addLine("gen", "material", matName);
	}
//	else if (!Fl::event_state(FL_BUTTONS) && (w == ui->xLocSlider || w == ui->yLocSlider || w == ui->zLocSlider))
//	{
//		scriptMgr->addLine("xform", "loc", xLocSlider->get_parm_ptr()->get(), yLocSlider->get_parm_ptr()->get(), zLocSlider->get_parm_ptr()->get());
//	}
	else if ( w == outputEnableButton )
	{
		geomPtr->setOutputFlag( outputEnableButton->value() );
	}
	else if ( w == outputNameChoice )
	{
		geomPtr->setOutputNameID( outputNameChoice->value() );
		nameInput->value( geomPtr->getName().get_char_star() );
		setTitle( geomPtr->getName().get_char_star() );
		screenMgrPtr->update( GEOM_SCREEN );
	}
	else if ( w == shellEnableButton )
	{
		geomPtr->setShellFlag( shellEnableButton->value() );
	}
	else if ( w == priorityCounter )
	{
		geomPtr->setMassPrior( (int)priorityCounter->value() );
	}

}

void CompScreen::setColor( int r, int g, int b )
{
	redSlider->value(r);
	greenSlider->value(g);
	blueSlider->value(b);

	geomPtr->setColor( r, g, b );

	primColorButton->color( fl_rgb_color(r, g, b) );
	primColorButton->redraw();
}

//****************************************************************************
//****************************************************************************
//****************************************************************************

//==== Constructor =====//
PodScreen::PodScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	PodUI* ui = podUI = new PodUI();

	podUI->UIWindow->position( 760, 30 );
	podUI->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	//==== Pod Stuff ====//
	lengthSlider = new Slider_adj_range(this, ui->lengthSlider, ui->lengthButtonL, ui->lengthButtonR, 10.0 );
	lengthInput  = new Input( this, ui->lengthInput );
	fineSlider = new Slider_adj_range(this, ui->fineSlider, ui->fineButtonL, ui->fineButtonR, 10.0 );
	fineInput  = new Input( this, ui->fineInput );

	lengthButton = new ParmButton( this, ui->lengthButton );
	fineButton = new ParmButton( this, ui->fineButton );

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
PodScreen::~PodScreen()
{
}

void PodScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (PodGeom*)geomPtr;
	PodUI* ui = podUI;

	lengthSlider->set_parm_ptr( &currGeom->length );
	lengthInput->set_parm_ptr( &currGeom->length );

	fineSlider->set_parm_ptr( &currGeom->fine_ratio);
	fineInput->set_parm_ptr( &currGeom->fine_ratio );

	lengthButton->set_parm_ptr( &currGeom->length );
	fineButton->set_parm_ptr( &currGeom->fine_ratio);

	setTitle( currGeom->getName().get_char_star() );

	ui->UIWindow->show();

}

void PodScreen::hide()
{
	podUI->UIWindow->hide();
}

void PodScreen::position( int x, int y )
{
	podUI->UIWindow->position( x, y );
}

void PodScreen::setTitle( const char* name )
{
	title = "POD : ";
	title.concatenate( name );

	podUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void PodScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	podUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void PodScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

}

//****************************************************************************
//****************************************************************************
//****************************************************************************

//==== Constructor =====//
XsecGeomScreen::XsecGeomScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	XsecGeomUI* ui = xsecGeomUI = new XsecGeomUI();

	xsecGeomUI->UIWindow->position( 760, 30 );
	xsecGeomUI->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"
	#include "CommonParmButtonGui.h"

	//==== Pod Stuff ====//
	xScaleSlider = new Slider_adj_range(this, ui->xScaleSlider, ui->xScaleButtonL, ui->xScaleButtonR, 1.0 );
	xScaleInput  = new Input( this, ui->xScaleInput );
	yScaleSlider = new Slider_adj_range(this, ui->yScaleSlider, ui->yScaleButtonL, ui->yScaleButtonR, 1.0 );
	yScaleInput  = new Input( this, ui->yScaleInput );
	zScaleSlider = new Slider_adj_range(this, ui->zScaleSlider, ui->zScaleButtonL, ui->zScaleButtonR, 1.0 );
	zScaleInput  = new Input( this, ui->zScaleInput );
	uTanLimitInput  = new Input( this, ui->uTanLimitInput );
	wTanLimitInput  = new Input( this, ui->wTanLimitInput );



	//==== Initial General Comp Screen UI ====//
	CompScreen::init();

}

//==== Destructor =====//
XsecGeomScreen::~XsecGeomScreen()
{
}

void XsecGeomScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (XSecGeom*)geomPtr;

	xScaleSlider->set_parm_ptr( &currGeom->xScale );
	xScaleInput->set_parm_ptr( &currGeom->xScale );
	yScaleSlider->set_parm_ptr( &currGeom->yScale );
	yScaleInput->set_parm_ptr( &currGeom->yScale );
	zScaleSlider->set_parm_ptr( &currGeom->zScale );
	zScaleInput->set_parm_ptr( &currGeom->zScale );
	uTanLimitInput->set_parm_ptr( &currGeom->uAngle );
	wTanLimitInput->set_parm_ptr( &currGeom->wAngle );


	setTitle( currGeom->getName().get_char_star() );
	update_dims();

	xsecGeomUI->UIWindow->show();
}

void XsecGeomScreen::parm_changed( Parm* parm )
{
	if ( parm )
	{
		update_dims();
	}
}

void XsecGeomScreen::update_dims()
{
	if ( currGeom )
	{
		char str[256];
		bbox box = currGeom->get_bnd_box();

		sprintf( str, "%6.3f", box.get_min(0) ); 
		xsecGeomUI->xMinOutput->value(str);
		sprintf( str, "%6.3f", box.get_min(1) ); 
		xsecGeomUI->yMinOutput->value(str);
		sprintf( str, "%6.3f", box.get_min(2) ); 
		xsecGeomUI->zMinOutput->value(str);
		sprintf( str, "%6.3f", box.get_max(0) ); 
		xsecGeomUI->xMaxOutput->value(str);
		sprintf( str, "%6.3f", box.get_max(1) ); 
		xsecGeomUI->yMaxOutput->value(str);
		sprintf( str, "%6.3f", box.get_max(2) ); 
		xsecGeomUI->zMaxOutput->value(str);
	}
}


void XsecGeomScreen::hide()
{
	xsecGeomUI->UIWindow->hide();
}

void XsecGeomScreen::position( int x, int y )
{
	xsecGeomUI->UIWindow->position( x, y );
}

void XsecGeomScreen::setTitle( const char* name )
{
	title = "XSEC GEOM : ";
	title.concatenate( name );
	xsecGeomUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void XsecGeomScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	xsecGeomUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

//==== Handle Callbacks =====//
void XsecGeomScreen::screenCB( Fl_Widget* w)
{
	if ( !currGeom )
		return;

	CompScreen::screenCB( w );

}


