// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "curveEditScreen.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CurveEditScreen::CurveEditScreen(ScreenMgr* mgr, Aircraft* airPtr) : VspScreen( mgr )
{

	screenMgrPtr = mgr;
	aircraftPtr = airPtr;

	jpgImgData.data = 0;

	CurveEditUI* ui = curveEditUI =  new CurveEditUI();

	ui->UIWindow->position( 750, 450 );

	editCurvePtr = 0;

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->glWinGroup;
	ui->glWinGroup->begin();
	glWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->glWinGroup->end();


	xPntSlider = new Slider_adj_range(this, ui->xSlider, ui->xButtonL, ui->xButtonR, 4.0 );
	yPntSlider = new Slider_adj_range(this, ui->ySlider, ui->yButtonL, ui->yButtonR, 4.0 );
	xPntInput  = new Input( this, ui->xInput );
	yPntInput  = new Input( this, ui->yInput );

	scaleTanSlider = new Slider_adj_range(this, ui->scaleTansSlider, ui->scaleTansButtonL, ui->scaleTansButtonR, 4.0 );
	scaleTanInput  = new Input( this, ui->scaleTansInput );

	scaleXSlider = new LogSlider(this, ui->scaleXSlider );
	scaleXInput  = new Input( this, ui->scaleXInput );
	scaleYSlider = new LogSlider(this, ui->scaleYSlider );
	scaleYInput  = new Input( this, ui->scaleYInput );

	widthInput  = new Input( this, ui->maxWInput );
	heightInput = new Input( this, ui->maxHInput );

	ui->scaleXNo->callback( staticScreenCB, this );
	ui->scaleXOk->callback( staticScreenCB, this );
	ui->scaleYNo->callback( staticScreenCB, this );
	ui->scaleYOk->callback( staticScreenCB, this );

	ui->closeButton->callback( staticScreenCB, this );
	ui->smoothButton->callback( staticScreenCB, this );
	ui->addButton->callback( staticScreenCB, this );
	ui->delButton->callback( staticScreenCB, this );

	ui->symmetryChoice->callback( staticScreenCB, this );
	ui->lockXButton->callback( staticScreenCB, this );
	ui->lockYButton->callback( staticScreenCB, this );

	ui->shapeChoice->callback( staticScreenCB, this );
	ui->initButton->callback( staticScreenCB, this );

	ui->zoomRoller->callback( staticScreenCB, this );
	ui->backgroundButton->callback( staticScreenCB, this );

	
//	ui->pasteGeomButton->shortcut(FL_CTRL + 'v');

}

CurveEditScreen::~CurveEditScreen()
{

}

void CurveEditScreen::update()
{
	glWin->redraw();

}

void CurveEditScreen::parm_changed( Parm* parm )
{
	glWin->redraw();
}

void  CurveEditScreen::setEditCurvePtr( EditCurve* ptr )
{
	editCurvePtr = ptr;
	editCurvePtr->setCurveEditScreenPtr(this);

//jrg del old copy....
}

void CurveEditScreen::show(Geom*)		//jrg fix to pass in curve..
{
	show();			
}

void CurveEditScreen::show()
{
	if ( !editCurvePtr )
		return;

	xPntSlider->set_parm_ptr( editCurvePtr->getPntX() );
	yPntSlider->set_parm_ptr( editCurvePtr->getPntY() );

	xPntInput->set_parm_ptr( editCurvePtr->getPntX() );
	yPntInput->set_parm_ptr( editCurvePtr->getPntY() );

	scaleTanSlider->set_parm_ptr( editCurvePtr->getScaleTans() );
	scaleTanInput->set_parm_ptr( editCurvePtr->getScaleTans() );

	scaleXSlider->set_parm_ptr( editCurvePtr->getScaleX() );
	scaleYSlider->set_parm_ptr( editCurvePtr->getScaleY() );
	scaleXInput->set_parm_ptr( editCurvePtr->getScaleX() );
	scaleYInput->set_parm_ptr( editCurvePtr->getScaleY() );

	if ( editCurvePtr->getCloseFlag() )
		curveEditUI->closeButton->value(1);
	else
		curveEditUI->closeButton->value(0);

	if ( editCurvePtr->getSmoothFlag() )
		curveEditUI->smoothButton->value(1);
	else
		curveEditUI->smoothButton->value(0);

	curveEditUI->symmetryChoice->value( editCurvePtr->getSym() );
	curveEditUI->lockXButton->value( editCurvePtr->getLockX() );
	curveEditUI->lockYButton->value( editCurvePtr->getLockY() );

	widthInput->set_parm_ptr( editCurvePtr->getMaxWidth() );
	heightInput->set_parm_ptr( editCurvePtr->getMaxHeight() );
	
	curveEditUI->shapeChoice->value( editCurvePtr->getShapeType() );

	curveEditUI->zoomRoller->value( editCurvePtr->getRollerScaleFactor() );

	glWin->setDrawBase( editCurvePtr );
	glWin->redraw();

	curveEditUI->UIWindow->show();
}

void CurveEditScreen::hide()
{
	curveEditUI->UIWindow->hide();
}

void CurveEditScreen::setTitle( const char* name )
{
}

//==== Close Callbacks =====//
void CurveEditScreen::closeCB( Fl_Widget* w)
{
//jrg	currGeom->deactivate();
	curveEditUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );	//jrg ????
}

void CurveEditScreen::screenCB( Fl_Widget* w )
{
	if ( !editCurvePtr )
		return;

	if ( w == curveEditUI->closeButton )
	{
		editCurvePtr->setCloseFlag( curveEditUI->closeButton->value() );
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->smoothButton )
	{
		editCurvePtr->setSmoothFlag( curveEditUI->smoothButton->value() );
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->addButton )
	{
		editCurvePtr->addEditPnt();
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->delButton )
	{
		editCurvePtr->delEditPnt();
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->symmetryChoice )
	{
		int val = curveEditUI->symmetryChoice->value();
		editCurvePtr->setSym(val);
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->lockXButton )
	{
		int val = curveEditUI->lockXButton->value();
		editCurvePtr->setLockX(val);
		glWin->redraw();
	}
	else if ( w == curveEditUI->lockYButton )
	{
		int val = curveEditUI->lockYButton->value();
		editCurvePtr->setLockY(val);
		glWin->redraw();
	}
	else if ( w == curveEditUI->scaleXNo )
	{
		editCurvePtr->resetScaleX();
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->scaleXOk )
	{
		editCurvePtr->acceptScaleX();
		glWin->redraw();
	}
	else if ( w == curveEditUI->scaleYNo )
	{
		editCurvePtr->resetScaleY();
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->scaleYOk )
	{
		editCurvePtr->acceptScaleY();
		glWin->redraw();
	}
	else if ( w == curveEditUI->initButton  )
	{
		editCurvePtr->initShape();
		editCurvePtr->generate();
		editCurvePtr->triggerGeom();
		glWin->redraw();
	}
	else if ( w == curveEditUI->shapeChoice  )
	{
		int val = curveEditUI->shapeChoice->value();
		editCurvePtr->setShapeType( val );
	}
	else if ( w == curveEditUI->zoomRoller )
	{
		double val = curveEditUI->zoomRoller->value();
		editCurvePtr->setDrawScaleFactor( val );
		glWin->redraw();
	}
	else if ( w == curveEditUI->backgroundButton  )
	{
		if ( curveEditUI->backgroundButton->value() )
		{

//			char *newfile = fl_file_chooser("Read Deflection File?", "*.jpg", "");
			char *newfile = screenMgrPtr->FileChooser("Read Deflection File?", "*.jpg");
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

}



