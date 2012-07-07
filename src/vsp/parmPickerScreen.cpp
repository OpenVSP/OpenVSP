//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmPickerScreen.cpp: implementation of the parmPickerScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "parmPickerScreen.h"
#include "parmLinkMgr.h"
#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmPickerScreen::ParmPickerScreen(ScreenMgr* mgr, Aircraft* airPtr) : VspScreen( mgr )
{
	aircraftPtr = airPtr;

	ParmPickerUI* ui = parmPickerUI = new ParmPickerUI();
	parmPickerUI->UIWindow->position( 780, 30 );

	ui->compChoice->callback( staticScreenCB, this );
	ui->groupChoice->callback( staticScreenCB, this );
	ui->parmChoice->callback( staticScreenCB, this );

	ui->addParmButton->callback( staticScreenCB, this );
	ui->deleteParmButton->callback( staticScreenCB, this );
	ui->parmBrowser->callback( staticScreenCB, this );

}

ParmPickerScreen::~ParmPickerScreen()
{

}

void ParmPickerScreen::RemoveAllRefs( GeomBase* gPtr )
{

}

void ParmPickerScreen::update()
{
	int i;
	char str[256];

	ParmHolder* currPHolder = pHolderListMgrPtr->GetCurrPHolder();

	if ( currPHolder->getParm() == NULL )
		currPHolder = pHolderListMgrPtr->ResetWorkingPHolder();

	//==== Geom Names ====//
	parmPickerUI->compChoice->clear();
	vector< string > geomNameVec;
	int ind = parmMgrPtr->GetCurrGeomNameVec( currPHolder->getParm(), geomNameVec );
	for ( i = 0 ; i < (int)geomNameVec.size() ; i++ )
	{
		sprintf( str, "%d-%s", i,  geomNameVec[i].c_str() );
		parmPickerUI->compChoice->add( str );
	}
		//parmPickerUI->compAChoice->add( geomNameVecA[i].c_str() );
	parmPickerUI->compChoice->value( ind );

	//==== Group Names ====//
	parmPickerUI->groupChoice->clear();
	vector< string > groupNameVec;
	ind = parmMgrPtr->GetCurrGroupNameVec( currPHolder->getParm(), groupNameVec );
	for ( i = 0 ; i < (int)groupNameVec.size() ; i++ )
		parmPickerUI->groupChoice->add( groupNameVec[i].c_str() );
	parmPickerUI->groupChoice->value( ind );

	//==== Parm Names =====//
	parmPickerUI->parmChoice->clear();
	vector< string > parmNameVec;
	ind = parmMgrPtr->GetCurrParmNameVec( currPHolder->getParm(), parmNameVec );
	for ( i = 0 ; i < (int)parmNameVec.size() ; i++ )
		parmPickerUI->parmChoice->add( parmNameVec[i].c_str() );
	parmPickerUI->parmChoice->value( ind );

	//==== Update Parm Browser ====//
	parmPickerUI->parmBrowser->clear();

	static int widths[] = { 75,75,90,20,75,75,80,0 };	// widths for each column
	parmPickerUI->parmBrowser->column_widths(widths);		// assign array to widget
	parmPickerUI->parmBrowser->column_char(':');			// use : as the column character

	sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM" );
	parmPickerUI->parmBrowser->add( str );

	vector< ParmHolder* > pHolderVec = pHolderListMgrPtr->GetPHolderVec();
	for ( i = 0 ; i < (int)pHolderVec.size() ; i++ )
	{
		ParmHolder* ph = pHolderVec[i];
		sprintf( str, "%s:%s:%s",
			ph->getParm()->get_geom_base()->getName().get_char_star(), ph->getParm()->get_group_name().get_char_star(), ph->getParm()->get_name().get_char_star() );
		parmPickerUI->parmBrowser->add( str );
	}

	int index = pHolderListMgrPtr->GetCurrPHolderIndex();
	if ( index >= 0 && index < (int)pHolderVec.size() )
		parmPickerUI->parmBrowser->select( index+2 );

	parmPickerUI->UIWindow->redraw();

}


void ParmPickerScreen::show()
{
	show( aircraftPtr->getUserGeom() );
}

void ParmPickerScreen::show( Geom* geomPtr )
{
	parmMgrPtr->LoadAllParms();
	update();
	parmPickerUI->UIWindow->show();
}

void ParmPickerScreen::hide()
{
	parmPickerUI->UIWindow->hide();
}

void ParmPickerScreen::setTitle( const char* name )
{

}

//==== Close Callbacks =====//
void ParmPickerScreen::closeCB( Fl_Widget* w)
{
	parmPickerUI->UIWindow->hide();
}

void ParmPickerScreen::screenCB( Fl_Widget* w )
{
	ParmHolder* currPHolder = pHolderListMgrPtr->GetCurrPHolder();
	if ( w == parmPickerUI->compChoice  ||
		 w == parmPickerUI->groupChoice ||
		 w == parmPickerUI->parmChoice )
	{
		compGroupChange();
	}
	else if (  w == parmPickerUI->addParmButton )
	{
		bool success = pHolderListMgrPtr->AddCurrPHolder();
		if ( !success )
			fl_alert( "Error: Identical Parameter In List" );
		update();
	}
	else if (  w == parmPickerUI->deleteParmButton )
	{
		pHolderListMgrPtr->DelCurrPHolder();
		update();
	}
	else if (  w == parmPickerUI->parmBrowser )
	{
		int sel = parmPickerUI->parmBrowser->value();
		pHolderListMgrPtr->SetCurrPHolderIndex( sel-2 );
		update();
	}

	aircraftPtr->triggerDraw();
}

void ParmPickerScreen::compGroupChange()
{
	ParmPickerUI* ui = parmPickerUI;
	pHolderListMgrPtr->SetCurrPHolderIndex(-1);
	pHolderListMgrPtr->SetParm( ui->compChoice->value(), ui->groupChoice->value(), ui->parmChoice->value() );
	update();
}
