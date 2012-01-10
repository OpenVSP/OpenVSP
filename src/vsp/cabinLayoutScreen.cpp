
#include "cabinLayoutScreen.h"
#include "cabinLayoutGeom.h"
#include "scriptMgr.h"
#include <FL/Fl_File_Chooser.H>
#include "cabinLayoutFLScreen.h"

CabinLayoutScreen::CabinLayoutScreen(ScreenMgr* mgr) : CompScreen( mgr )
{
	CabinLayoutUI* ui = cabinLayoutUI = new CabinLayoutUI();

	cabinLayoutUI->UIWindow->position( 760, 30 );
	cabinLayoutUI->UIWindow->callback( staticCloseCB, this );

	//==============================================// 
	//==== Standard Geom Stuff - IN COMP SCREEN ====//
	#include "CommonCompGui.h"

	//==== Hermite Stuff ====//
	ui->loadHermiteFileButton->callback( staticScreenCB, this );
	ui->mirrorDataButton->callback( staticScreenCB, this );

	//==== Initail General Comp Screen UI ====//
	CompScreen::init();
}

CabinLayoutScreen::~CabinLayoutScreen()
{

}

void CabinLayoutScreen::show(Geom* geomPtr)
{
	//==== Show General Component Screen ====//
	CompScreen::show(geomPtr);

	currGeom = (CabinLayoutGeom*)geomPtr;
	CabinLayoutUI* ui = cabinLayoutUI;

	setTitle( currGeom->getName().get_char_star() );
	ui->hermiteFilePathOutput->value(currGeom->get_geom_data_file_name().get_char_star());
	ui->mirrorDataButton->value(currGeom->get_MirrorData());

	ui->UIWindow->show();

	numXsecsSlider->deactivate();
	numXsecsInput->deactivate();
	numPntsSlider->deactivate();
	numPntsInput->deactivate();
}

void CabinLayoutScreen::screenCB(Fl_Widget* w)
{
	if(w == cabinLayoutUI->loadHermiteFileButton)
	{
//		const char* newfile = fl_file_chooser("Choose Cabin Layout File?", "*.*", "");
		const char* newfile = screenMgrPtr->FileChooser("Choose Cabin Layout File?", "*.*" );

		if(newfile != NULL)
		{
			cabinLayoutUI->hermiteFilePathOutput->value(newfile);
			currGeom->set_geom_data_file_name(newfile);
		}
	}
	else if(w == cabinLayoutUI->mirrorDataButton)
	{
		if(cabinLayoutUI->mirrorDataButton->value())
		{
			currGeom->set_MirrorData(true);
		}
		else
		{
			currGeom->set_MirrorData(false);
		}
	}

	CompScreen::screenCB( w );
}

void CabinLayoutScreen::closeCB( Fl_Widget* w)
{
	currGeom->deactivate();
	cabinLayoutUI->UIWindow->hide();
	screenMgrPtr->update( GEOM_SCREEN );
}

void CabinLayoutScreen::setTitle( const char* name )
{
	title = "CABIN LAYOUT : ";
	title.concatenate( name );

	cabinLayoutUI->TitleBox->label( title );
}

void CabinLayoutScreen::position( int x, int y )
{
	cabinLayoutUI->UIWindow->position( x, y );
}

void CabinLayoutScreen::hide()
{
	cabinLayoutUI->UIWindow->hide();
}

