//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "geomScreen.h"
#include "groupScreen.h"
#include "scriptMgr.h"
#include "VspPreferences.h"
#include "parmLinkMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GeomScreen::GeomScreen(ScreenMgr* mgr, Aircraft* airPtr)
{
	screenMgrPtr = mgr;
	aircraftPtr = airPtr;
	lastSelectedGeom = NULL;
	lastTopLine = 0;
	collaspeFlag = false;

	GeomUI* ui = geomUI = new GeomUI();

	ui->geomBrowser->callback( staticScreenCB, this );
	ui->addGeomButton->callback( staticScreenCB, this );
	ui->cutGeomButton->callback( staticScreenCB, this );
	ui->copyGeomButton->callback( staticScreenCB, this );
	ui->pasteGeomButton->callback( staticScreenCB, this );
	ui->noshowGeomButton->callback( staticScreenCB, this );
	ui->showGeomButton->callback( staticScreenCB, this );
	ui->selectAllGeomButton->callback( staticScreenCB, this );
	ui->activeGeomInput->callback( staticScreenCB, this );

	ui->wireGeomButton->callback( staticScreenCB, this );
	ui->shadeGeomButton->callback( staticScreenCB, this );
	ui->hiddenGeomButton->callback( staticScreenCB, this );
	ui->textureGeomButton->callback( staticScreenCB, this );

	ui->moveUpButton->callback( staticScreenCB, this );
	ui->moveDownButton->callback( staticScreenCB, this );
	ui->moveTopButton->callback( staticScreenCB, this );
	ui->moveBotButton->callback( staticScreenCB, this );

	ui->defaultAircraftTypeChoice->callback( staticScreenCB, this );

	ui->cutGeomButton->shortcut(FL_CTRL + 'x');
	ui->copyGeomButton->shortcut(FL_CTRL + 'c');
	ui->pasteGeomButton->shortcut(FL_CTRL + 'v');

}

GeomScreen::~GeomScreen()
{

}

void GeomScreen::update()
{
	//==== Load Browser ====//
	loadGeomBrowser();
//	geomUI->geomBrowser->deselect();
	loadActiveGeomOutput();
}

void GeomScreen::deselectGeomBrowser()
{
	geomUI->geomBrowser->deselect();
}


void GeomScreen::show()
{
	//==== Load Browser ====//
	loadGeomBrowser();

	//==== Load Default Aircraft Menu ====//
	while ( geomUI->defaultAircraftTypeChoice->size() > 1 )
		geomUI->defaultAircraftTypeChoice->remove(0);

	vector< DefaultCompFile > defCompFileVec = VspPreferences::Instance()->getDefaultCompFileVec();
	for ( int i = 0 ; i < (int)defCompFileVec.size() ; i++ )
	{
		geomUI->defaultAircraftTypeChoice->add( defCompFileVec[i].m_Name.get_char_star() );
	}

	int curr_id = VspPreferences::Instance()->getDefaultCompFileID();
	geomUI->defaultAircraftTypeChoice->value( curr_id );
	geomUI->UIWindow->show();
}

void GeomScreen::hide()
{
	geomUI->UIWindow->hide();
}

void GeomScreen::loadGeomBrowser()
{
	int i;

	//==== Save List of Selected Geoms ====//
	vector< Geom* > selVec = getSelectedGeomVec();

	lastTopLine = geomUI->geomBrowser->topline();

	//==== Get Geoms To Display ====//
	vector< Geom* > geomVec = aircraftPtr->getDisplayGeomVec();
//	vector< Geom* > geomVec = aircraftPtr->getGeomVec();

	geomUI->geomBrowser->clear();
	geomUI->geomBrowser->add( aircraftPtr->getName() );

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		Stringc str; 
		//==== Check if Parent is Selected ====//
		if ( isParentSelected( geomVec[i], selVec ) )
			str.concatenate("@b@.");

		int numindents = geomVec[i]->countParents(0);
		for ( int j = 0 ; j < numindents ; j++ )
		{
			str.concatenate("--");
		}
		if ( geomVec[i]->getPosAttachFlag() == POS_ATTACH_NONE )
			str.concatenate("> ");
		else
			str.concatenate("^ ");

		if ( !geomVec[i]->getDisplayChildrenFlag() )
			str.concatenate("(+) ");

		str.concatenate(geomVec[i]->getName()); 

		if ( geomVec[i]->getNoShowFlag() )
			str.concatenate("(no show)");

		geomUI->geomBrowser->add( str );
	}

	//==== Restore List of Selected Geoms ====//
	for ( i = 0 ; i < (int)selVec.size() ; i++ )
		selectGeomBrowser( selVec[i] );


}

bool GeomScreen::isParentSelected( Geom* geomPtr, vector< Geom* > & selVec )
{
	Geom* checkGeom = geomPtr;
	while ( checkGeom )
	{
		for ( int i = 0 ; i < (int)selVec.size() ; i++ )
			if ( checkGeom == selVec[i] )
				return true;

		checkGeom = checkGeom->getParent();
	}
	return false;
}

void GeomScreen::loadActiveGeomOutput()
{
	vector< Geom* > activeVec = aircraftPtr->getActiveGeomVec();
	if ( activeVec.size() == 0)
		geomUI->activeGeomInput->value( aircraftPtr->getName() );
	else if (activeVec.size() == 1)
		geomUI->activeGeomInput->value( activeVec[0]->getName() );
	else
		geomUI->activeGeomInput->value( "<multiple>" );


}

void GeomScreen::selectGeomBrowser( Geom* geom )
{
//	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	vector< Geom* > geomVec = aircraftPtr->getDisplayGeomVec();

	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i] == geom )
			geomUI->geomBrowser->select( i+2 );
	}

	//==== Position Browser ====//
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i] == geom )
		{
			geomUI->geomBrowser->topline( i + 2 );
			break;
		}
	}

	if ( !collaspeFlag && lastTopLine < ((int)geomVec.size()-2) )
		geomUI->geomBrowser->topline( lastTopLine );

}


void GeomScreen::screenCB( Fl_Widget* w )
{

	if ( w == geomUI->addGeomButton )
	{
		int type = geomUI->geomTypeChoice->value();

		if ( type >= WING_GEOM_TYPE )			// Compensate for removing Wing Type
			type += 1;

		s_add_geom(ScriptMgr::GUI, type, Stringc(""));
	}
	else if ( w == geomUI->geomBrowser)
	{
		s_select_gui();
	}
	else if ( w == geomUI->noshowGeomButton )
	{
		vector<Geom*> selVec = getSelectedGeomVec();

		for ( int i = 0 ; i < (int)selVec.size() ; i++ )
		{
			selVec[i]->setNoShowFlag( 1 );
		}
		aircraftPtr->triggerDraw();
		loadGeomBrowser();
	}
	else if ( w == geomUI->showGeomButton )
	{
		vector<Geom*> selVec = getSelectedGeomVec();

		for ( int i = 0 ; i < (int)selVec.size() ; i++ )
		{
			selVec[i]->setNoShowFlag( 0 );
		}
		aircraftPtr->triggerDraw();
		loadGeomBrowser();

	}
	else if ( w == geomUI->selectAllGeomButton )
	{
		s_select_all(ScriptMgr::GUI);
	}
	else if ( w == geomUI->activeGeomInput )
	{
		Geom* activeGeom = aircraftPtr->getActiveGeom();
		if ( activeGeom )
		{
			Stringc name = geomUI->activeGeomInput->value();
			activeGeom->setName( name );
			loadGeomBrowser();

			name.quote('\"');
			scriptMgr->addLine("gen", "name", name);		}
		else
		{
			aircraftPtr->setName( geomUI->activeGeomInput->value() );
			loadGeomBrowser();
		}

		vector<Geom*> selVec = getSelectedGeomVec();

		if ( selVec.size() == 1 )
			aircraftPtr->modifyGeom( selVec[0] );
	}
	else if ( w == geomUI->cutGeomButton )
	{
		s_cut(ScriptMgr::GUI, deque< Stringc >());
	}
	else if ( w == geomUI->copyGeomButton )
	{
		s_copy(ScriptMgr::GUI, deque< Stringc >());
	}
	else if ( w == geomUI->pasteGeomButton )
	{
		s_paste(ScriptMgr::GUI);
	}
	else if ( w == geomUI->wireGeomButton )
	{
		setGeomDisplayFlag( GEOM_WIRE_FLAG );
	}
	else if ( w == geomUI->hiddenGeomButton )
	{
		setGeomDisplayFlag( GEOM_HIDDEN_FLAG );
	}
	else if ( w == geomUI->shadeGeomButton )
	{
		setGeomDisplayFlag( GEOM_SHADE_FLAG );
	}
	else if ( w == geomUI->textureGeomButton )
	{
		setGeomDisplayFlag( GEOM_TEXTURE_FLAG );
	}
	else if ( w == geomUI->moveUpButton )
	{
		aircraftPtr->reorderGeom( Aircraft::MOVE_UP );
		deselectGeomBrowser();
			
		Geom * activeGeom = aircraftPtr->getActiveGeom();
		if ( activeGeom )
			selectGeomBrowser(activeGeom);
		loadGeomBrowser();
	}
	else if ( w == geomUI->moveDownButton )
	{
		aircraftPtr->reorderGeom( Aircraft::MOVE_DOWN );
		deselectGeomBrowser();

		Geom * activeGeom = aircraftPtr->getActiveGeom();
		if ( activeGeom )
			selectGeomBrowser(activeGeom);
		loadGeomBrowser();
	}
	else if ( w == geomUI->moveTopButton )
	{
		aircraftPtr->reorderGeom( Aircraft::MOVE_TOP );
		deselectGeomBrowser();
			
		Geom * activeGeom = aircraftPtr->getActiveGeom();
		if ( activeGeom )
			selectGeomBrowser(activeGeom);
		loadGeomBrowser();
	}
	else if ( w == geomUI->moveBotButton )
	{
		aircraftPtr->reorderGeom( Aircraft::MOVE_BOT );
		deselectGeomBrowser();
			
		Geom * activeGeom = aircraftPtr->getActiveGeom();
		if ( activeGeom )
			selectGeomBrowser(activeGeom);
		loadGeomBrowser();
	}
	else if ( w == geomUI->defaultAircraftTypeChoice )
	{
		int id = geomUI->defaultAircraftTypeChoice->value();
		VspPreferences::Instance()->setDefaultCompFileID( id );
		aircraftPtr->setDefaultCompGroupID(id);
	}

}

	
void GeomScreen::setGeomDisplayFlag( int flag )
{
	vector<Geom*> selVec = getSelectedGeomVec();
	for ( int i = 0 ; i < (int)selVec.size() ; i++ )
	{
		selVec[i]->setDisplayFlag( flag );
		if ( !selVec[i]->getDisplayChildrenFlag() )
		{
			vector<Geom*> childVec;
			selVec[i]->loadChildren( childVec );
			for ( int j = 0 ; j < (int)childVec.size() ; j++ )
				childVec[j]->setDisplayFlag( flag );
		}
	}
	aircraftPtr->triggerDraw();
}


vector< Geom* > GeomScreen::getSelectedGeomVec()
{
	vector<Geom*> selVec;
//	vector< Geom* > geomVec = aircraftPtr->getGeomVec();
	vector< Geom* > geomVec = aircraftPtr->getDisplayGeomVec();

	//==== Account For Browser Lines Starting at 1 and Aircraft Name ====//
	for ( int i = 2 ; i <= geomUI->geomBrowser->size() ; i++ )
	{
		if ( geomUI->geomBrowser->selected(i) && (int)geomVec.size() > (i-2) )
		{
			selVec.push_back( geomVec[i-2] );	
		}
	}

	return selVec;
}

Stringc GeomScreen::getNameString(vector< Geom* > selVec)
{
	char cs[256];

	Stringc str("");
	for ( int i = 0; i < (int)selVec.size(); i++) {
		sprintf(cs, "\"%s\" ", selVec[i]->getName()());
		str.concatenate(cs);
	}
	return str;
}





vector< Geom* > GeomScreen::s_get_geom(deque< Stringc > selectVec)
{
	vector< Geom * > geomVec;
	for (int i = 0; i < (int)selectVec.size(); i++)
	{	// select geoms
		Stringc name = selectVec[i];
		name.trim('\"');
		vector<Geom *> gVec;

		if (name.get_length() > 0 && name[0] == '+')
		{	// + indicates children selection
			name.remove_leading('+');
			gVec = aircraftPtr->getGeomByName(name, 1);
		}
		else
		{
			gVec = aircraftPtr->getGeomByName(name, 0);
		}

		for (int j = 0; j < (int)gVec.size(); j++)
		{	// add all geoms that are not already in the list
			vector< Geom* >::iterator giter = find(geomVec.begin(), geomVec.end(), gVec[j]);
			if (giter == geomVec.end())
				geomVec.push_back(gVec[j]);
		}
	}
	return geomVec;
}


void GeomScreen::s_add_geom(int src, int type, Stringc name)
{
	Geom* addGeom = 0;
	lastSelectedGeom = NULL;

	if ( type == POD_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(POD_GEOM_TYPE);
	}
	else if ( type == WING_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(WING_GEOM_TYPE);
	}
	else if ( type == FUSE_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(FUSE_GEOM_TYPE);
	}
	else if ( type == HAVOC_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(HAVOC_GEOM_TYPE);
	}
	else if ( type == EXT_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(EXT_GEOM_TYPE);
	}
	else if ( type == MS_WING_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(MS_WING_GEOM_TYPE);
	}
	else if ( type == BLANK_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(BLANK_GEOM_TYPE);
	}
	else if ( type == DUCT_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(DUCT_GEOM_TYPE);
	}
	else if ( type == PROP_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(PROP_GEOM_TYPE);
	}
	else if ( type == ENGINE_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(ENGINE_GEOM_TYPE);
	}
	else if ( type == HWB_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(HWB_GEOM_TYPE);
	}
	else if ( type == FUSELAGE_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(FUSELAGE_GEOM_TYPE);
	}
	else if ( type == CABIN_LAYOUT_GEOM_TYPE )
	{
		addGeom = aircraftPtr->createGeom(CABIN_LAYOUT_GEOM_TYPE);
	}

	if (addGeom && name.get_length() > 0)
	{
		addGeom->setName(name);
	}

	if (src != ScriptMgr::SCRIPT)
	{
		loadGeomBrowser();
		deselectGeomBrowser();

		if ( addGeom )
			selectGeomBrowser(addGeom);

		loadActiveGeomOutput();
		screenMgrPtr->updateGeomScreens();
		aircraftPtr->flagActiveGeom();
		loadGeomBrowser();

		if ( addGeom )
		{
			if (src == ScriptMgr::GUI) scriptMgr->addLine("add", addGeom->getTypeStr(), addGeom->getName());
		}
	}
}

void GeomScreen::s_cut(int src, deque< Stringc > partVec)
{
//	if (src != ScriptMgr::SCRIPT)
//		screenMgrPtr->hideGeomScreens();
	lastSelectedGeom = NULL;

	vector<Geom*> selVec;
	if (partVec.size() == 0)
		selVec = getSelectedGeomVec();
	else
		selVec = s_get_geom(partVec);


	aircraftPtr->cutGeomVec( selVec );

	if (src != ScriptMgr::SCRIPT)
	{
		loadGeomBrowser();
		loadActiveGeomOutput();
		screenMgrPtr->updateGeomScreens();
		deselectGeomBrowser();
	}

	if (src == ScriptMgr::GUI) scriptMgr->addLine("cut");
}

void GeomScreen::s_copy(int src, deque< Stringc > partVec)
{
	lastSelectedGeom = NULL;
	vector<Geom*> selVec;
	if (partVec.size() == 0)
		selVec = getSelectedGeomVec();
	else
		selVec = s_get_geom(partVec);
	
	aircraftPtr->copyGeomVec( selVec );

	if (src == ScriptMgr::GUI) scriptMgr->addLine("copy", getNameString(selVec));
}

void GeomScreen::s_paste(int src)
{
	lastSelectedGeom = NULL;

	aircraftPtr->resetClipBoardPtrID();
	aircraftPtr->pasteClipBoard();

	if (src != ScriptMgr::SCRIPT)
	{
		loadGeomBrowser();
		deselectGeomBrowser();

		screenMgrPtr->hideGeomScreens();
		aircraftPtr->clearBBoxColors();
	}

	if (src == ScriptMgr::GUI) scriptMgr->addLine("paste");
}


void GeomScreen::s_select_gui() 
{
	vector<Geom*> selVec = getSelectedGeomVec();

	//==== Find Last Selected Geom ====//
	int last = geomUI->geomBrowser->value();
	if ( (last >= 2) && Fl::event_state(FL_ALT) )
	{	// select children
//		Geom* lastSel = aircraftPtr->getGeomVec()[last-2];
		Geom* lastSel = aircraftPtr->getDisplayGeomVec()[last-2];
		vector<Geom*> cVec;
		lastSel->loadChildren( cVec );
		for (int i = 0; i < (int)cVec.size(); i++)
		{
			selectGeomBrowser( cVec[i] );
			selVec.push_back(cVec[i]);
		}	
	}			

	//==== Check if Geom Already Selected ====//
	collaspeFlag = false;
	if ( lastSelectedGeom && selVec.size() == 1 )
	{
		Geom* lastSel = selVec[0];
		if ( lastSel == lastSelectedGeom )
		{
			collaspeFlag = true;
			lastSel->toggleDisplayChildrenFlag();
			if ( lastSel->getChildren().size() == 0 )
				lastSel->setDisplayChildrenFlag( 1 );
		}
	}
	lastSelectedGeom = NULL;
	if ( selVec.size() == 1 )
		lastSelectedGeom = selVec[0];


	aircraftPtr->setActiveGeomVec(selVec);
	loadActiveGeomOutput();
	screenMgrPtr->updateGeomScreens();
	aircraftPtr->flagActiveGeom();
	aircraftPtr->triggerDraw();
	loadGeomBrowser();

	if (selVec.size() == 0)
	{
		scriptMgr->addLine("select", "Aircraft");
		geomUI->geomBrowser->value(1);
	}
	else
	{
		scriptMgr->addLine("select", getNameString(selVec));
	}
}


void GeomScreen::s_select(int src, deque< Stringc > partVec)
{

	vector< Geom * > geomVec = s_get_geom(partVec);
	if (geomVec.size() == 0)
	{
		s_select_none(src);
	} 
	else
	{
		aircraftPtr->setActiveGeomVec(geomVec);

		if (src == ScriptMgr::GUI_SCRIPT)
		{
			deselectGeomBrowser();
			for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
			{	// select in browser
				selectGeomBrowser( geomVec[i] );
			}
			loadActiveGeomOutput();
			screenMgrPtr->updateGeomScreens();
			aircraftPtr->flagActiveGeom();
			aircraftPtr->triggerDraw();

		}
	}
}


void GeomScreen::s_select_all(int src) {

	aircraftPtr->setActiveGeomVec(aircraftPtr->getGeomVec());

	if (src != ScriptMgr::SCRIPT)
	{
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			selectGeomBrowser( geomVec[i] );
		}

		loadActiveGeomOutput();
		screenMgrPtr->updateGeomScreens();
		aircraftPtr->flagActiveGeom();
		aircraftPtr->triggerDraw();
		if (src == ScriptMgr::GUI) scriptMgr->addLine("select all");
	}

}

void GeomScreen::s_select_none(int src) {
	aircraftPtr->setActiveGeom(NULL);
	
	if (src != ScriptMgr::SCRIPT)
	{
		deselectGeomBrowser();			// select none
		screenMgrPtr->hideGeomScreens();
		screenMgrPtr->getGroupScreen()->hide();
		loadActiveGeomOutput();
		aircraftPtr->flagActiveGeom();
		aircraftPtr->triggerDraw();
		geomUI->geomBrowser->value(1);
	}
	if (src == ScriptMgr::GUI) scriptMgr->addLine("select none");
}

