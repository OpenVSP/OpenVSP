//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// groupScreen.cpp: implementation of the groupScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "groupScreen.h"
#include "screenMgr.h"
#include "materialMgr.h"
#include "scriptMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//==== Constructor =====//
GroupScreen::GroupScreen(ScreenMgr* mgr, Aircraft* airPtr) : VspScreen( mgr )
{
	GroupUI* ui = groupUI = new GroupUI();

	groupUI->UIWindow->position( 750, 350 );
	groupUI->UIWindow->callback( staticCloseCB, this );
	ui->acceptScaleButton->callback( staticScreenCB, this );
	ui->acceptXformButton->callback( staticScreenCB, this );
	ui->resetScaleButton->callback( staticScreenCB, this );
	ui->resetXformButton->callback( staticScreenCB, this );
	ui->materialChoice->callback( staticScreenCB, this );
	ui->scaleLocButton->callback( staticScreenCB, this );


	ui->redSlider->callback( staticScreenCB, this );
	ui->redSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	ui->greenSlider->callback( staticScreenCB, this );
	ui->greenSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	ui->blueSlider->callback( staticScreenCB, this );
	ui->blueSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

	ui->materialChoice->clear();
	int num = matMgrPtr->getNumMaterial();
	for ( int i = 0 ; i < num ; i++ )
	{
		Material* mat = matMgrPtr->getMaterial(i);
		if ( mat )
			ui->materialChoice->add( mat->name );
	}
	ui->materialChoice->callback( staticScreenCB, this );

	
	xLocSlider = new Slider_adj_range(this, ui->xLocSlider, ui->xLocButtonL, ui->xLocButtonR, 4.0 );
	xLocInput = new Input( this, ui->xLocInput );
	yLocSlider = new Slider_adj_range(this, ui->yLocSlider, ui->yLocButtonL, ui->yLocButtonR, 4.0 );
	yLocInput = new Input( this, ui->yLocInput );
	zLocSlider = new Slider_adj_range(this, ui->zLocSlider, ui->zLocButtonL, ui->zLocButtonR, 4.0 );
	zLocInput = new Input( this, ui->zLocInput );


	xRotSlider = new Slider_adj_range(this, ui->xRotSlider, ui->xRotButtonL, ui->xRotButtonR, 45.0 );
	xRotInput = new Input( this, ui->xRotInput );
	yRotSlider = new Slider_adj_range(this, ui->yRotSlider, ui->yRotButtonL, ui->yRotButtonR, 45.0 );
	yRotInput = new Input( this, ui->yRotInput );
	zRotSlider = new Slider_adj_range(this, ui->zRotSlider, ui->zRotButtonL, ui->zRotButtonR, 45.0 );
	zRotInput = new Input( this, ui->zRotInput );

	scaleSlider = new Slider(this, ui->scaleSlider );
	scaleInput  = new Input( this, ui->scaleInput );



	geomGroup = new GeomGroup( airPtr );		
}

GroupScreen::~GroupScreen()
{

}

void GroupScreen::show(vector< Geom* > gVec )
{
	geomVec = gVec;

	geomGroup->setGeomVec( gVec );

	vec3d c = geomGroup->getColor();
	groupUI->primColorButton->color( fl_rgb_color((int)c.x(), (int)c.y(), (int)c.z()) );

	groupUI->redSlider->value( c.x() );
	groupUI->greenSlider->value( c.y() );
	groupUI->blueSlider->value( c.z() );

	xLocSlider->set_parm_ptr( &geomGroup->xLocOff );
	xLocInput->set_parm_ptr( &geomGroup->xLocOff );
	yLocSlider->set_parm_ptr( &geomGroup->yLocOff );
	yLocInput->set_parm_ptr( &geomGroup->yLocOff );
	zLocSlider->set_parm_ptr( &geomGroup->zLocOff );
	zLocInput->set_parm_ptr( &geomGroup->zLocOff );

	xRotSlider->set_parm_ptr( &geomGroup->xRotOff );
	xRotInput->set_parm_ptr( &geomGroup->xRotOff );
	yRotSlider->set_parm_ptr( &geomGroup->yRotOff );
	yRotInput->set_parm_ptr( &geomGroup->yRotOff );
	zRotSlider->set_parm_ptr( &geomGroup->zRotOff );
	zRotInput->set_parm_ptr( &geomGroup->zRotOff );

	scaleSlider->set_parm_ptr( &geomGroup->scaleFactor );
	scaleInput->set_parm_ptr( &geomGroup->scaleFactor );

	groupUI->scaleLocButton->value( geomGroup->getScaleLocFlag() );
	
	groupUI->UIWindow->show();

}

void GroupScreen::hide()
{
	geomGroup->clear();

	geomGroup->resetXform();
	geomGroup->resetScaleFactor();
	geomGroup->clear();
	groupUI->UIWindow->hide();
}


void GroupScreen::screenCB( Fl_Widget* w )
{
	if ( w == groupUI->acceptScaleButton )
	{
		scriptMgr->addLineDouble("xform scale", geomGroup->scaleFactor());
		geomGroup->acceptScaleFactor();
	}
	else if ( w == groupUI->acceptXformButton )
	{
		scriptMgr->addLine("xform", "loc", geomGroup->xLocOff(), geomGroup->yLocOff(), geomGroup->zLocOff());
		scriptMgr->addLine("xform", "rot", geomGroup->xRotOff(), geomGroup->yRotOff(), geomGroup->zRotOff());

		geomGroup->acceptXform();
	}
	else if ( w == groupUI->resetScaleButton )
	{
		geomGroup->resetScaleFactor();
		scriptMgr->addLine("xform", "resetscale");
	}
	else if ( w == groupUI->resetXformButton )
	{
		geomGroup->resetXform();		
		scriptMgr->addLine("xform", "resetxform");
	}
	else if ( w == groupUI->redSlider || 
			  w == groupUI->greenSlider	||
			  w == groupUI->blueSlider )				
	{
		int r = (int)groupUI->redSlider->value();
		int g = (int)groupUI->greenSlider->value();
		int b = (int)groupUI->blueSlider->value();

		geomGroup->setColor( r, g, b );

		groupUI->primColorButton->color( fl_rgb_color(r, g, b) );
		groupUI->primColorButton->redraw();

		if (!Fl::event_state(FL_BUTTONS))
			scriptMgr->addLine("gen", "color", r, g, b);
	}
	else if ( w == groupUI->materialChoice )
	{
		geomGroup->setMaterialID( groupUI->materialChoice->value() );
		Stringc matName = matMgrPtr->getMaterial(groupUI->materialChoice->value())->name();
		matName.quote('\"');
//		char str[128];
//		sprintf(str, "\"%s\"", matMgrPtr->getMaterial(groupUI->materialChoice->value())->name());
		scriptMgr->addLine("gen", "material", matName);
	}
	else if ( w == groupUI->scaleLocButton )
	{
		geomGroup->setScaleLocFlag( groupUI->scaleLocButton->value() );
	}



}

void GroupScreen::closeCB( Fl_Widget* w )
{
	hide();
//	screenMgrPtr->update( GEOM_SCREEN );
}

void GroupScreen::position( int x, int y )
{
	groupUI->UIWindow->position( x, y );

}

void GroupScreen::setTitle( const char* name )
{
	title = "GROUP MODIFY: ";
	title.concatenate( name );

	groupUI->TitleBox->label( title );


}















