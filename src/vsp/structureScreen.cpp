//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "structureScreen.h"
#include "groupScreen.h"
#include "scriptMgr.h"
#include "structureMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructureScreen::StructureScreen(ScreenMgr* mgr, Aircraft* airPtr)
{
	screenMgrPtr = mgr;
	aircraftPtr = airPtr;

	StructureUI* ui = structureUI = new StructureUI();

	structureUI->UIWindow->position( 760, 30 );


	ui->compChoice->callback( staticScreenCB, this );
	ui->addStructureButton->callback( staticScreenCB, this );
	ui->u1Slider->callback( staticScreenCB, this );
	ui->w1Slider->callback( staticScreenCB, this );
	ui->u1Input->callback( staticScreenCB, this );
	ui->w1Input->callback( staticScreenCB, this );

	ui->u2Slider->callback( staticScreenCB, this );
	ui->w2Slider->callback( staticScreenCB, this );
	ui->u2Input->callback( staticScreenCB, this );
	ui->w2Input->callback( staticScreenCB, this );

	ui->u3Slider->callback( staticScreenCB, this );
	ui->w3Slider->callback( staticScreenCB, this );
	ui->u3Input->callback( staticScreenCB, this );
	ui->w3Input->callback( staticScreenCB, this );

	ui->finalMeshStructureButton->callback( staticScreenCB, this );
	ui->triAreaInput->callback( staticScreenCB, this );

	ui->deleteStructureButton->callback( staticScreenCB, this );
	ui->PartNameInput->callback( staticScreenCB, this );

	ui->structureBrowser->callback( staticScreenCB, this );

	ui->ribSpanSlider->callback( staticScreenCB, this );
	ui->ribSpanInput->callback( staticScreenCB, this );

	ui->sparChordSlider->callback( staticScreenCB, this );
	ui->sparChordInput->callback( staticScreenCB, this );

	ui->ExportNameInput->callback( staticScreenCB, this );

	ui->ResultsDisplay->buffer( new Fl_Text_Buffer() );

	Fl::add_timeout(1.0, staticTimerCB, this);



	currPartType = -1;



}

StructureScreen::~StructureScreen()
{

}

void StructureScreen::timerCB()
{

	structureMgrPtr->SetActiveFlag( !!structureUI->UIWindow->shown() );

	if ( !structureUI->UIWindow->shown() )
		return;

	if ( structureMgrPtr->GetUpdateFlag() )
		update();

	structureMgrPtr->OneSecondUpdate();
}


void StructureScreen::update()
{
	int i;
	char str[256];

	//==== Load Geom Choice ====//
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	

	structureUI->compChoice->clear();
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		structureUI->compChoice->add( geomVec[i]->getName() );
	}

	int currGeomID = structureMgrPtr->getCurrGeomID();
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		structureUI->compChoice->value( currGeomID );
	}


	//==== Get Current Part From Structure Mgr ====//
	Part* part = structureMgrPtr->GetCurrPart();

	if ( part )
	{
		structureUI->PartNameInput->value( part->GetName() );
		if ( part->GetType() == Part::THREE_PNT_SLICE )
		{
			ThreePntSlice* tps = (ThreePntSlice*)part;				// Cast
			vec2d p1 = tps->GetUWPnt(0);
			structureUI->u1Slider->value( p1.x() );
			structureUI->w1Slider->value( p1.y() );
			sprintf( str, "%4.3f", p1.x() );	structureUI->u1Input->value( str );
			sprintf( str, "%4.3f", p1.y() );	structureUI->w1Input->value( str );

			vec2d p2 = tps->GetUWPnt(1);
			structureUI->u2Slider->value( p2.x() );
			structureUI->w2Slider->value( p2.y() );
			sprintf( str, "%4.3f", p2.x() );	structureUI->u2Input->value( str );
			sprintf( str, "%4.3f", p2.y() );	structureUI->w2Input->value( str );

			vec2d p3 = tps->GetUWPnt(2);
			structureUI->u3Slider->value( p3.x() );
			structureUI->w3Slider->value( p3.y() );
			sprintf( str, "%4.3f", p3.x() );	structureUI->u3Input->value( str );
			sprintf( str, "%4.3f", p3.y() );	structureUI->w3Input->value( str );
		}
		else if ( part->GetType() == Part::RIB )
		{
			Rib* r = (Rib*)part;
			structureUI->ribSpanSlider->value( r->GetPercentSpan() );
			sprintf( str, "%4.3f", r->GetPercentSpan() );  structureUI->ribSpanInput->value( str );
		}
		else if ( part->GetType() == Part::SPAR )
		{
			Spar* s = (Spar*)part;
			structureUI->sparChordSlider->value( s->GetPercentChord() );
			sprintf( str, "%4.3f", s->GetPercentChord() );  structureUI->sparChordInput->value( str );
		}
	}

	//==== Load Up Part Browser ====//
	int currPartID = -1;
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		structureUI->structureBrowser->clear();

		vector< Part* > pVec = geomVec[currGeomID]->getStructurePartVec();


		for ( i = 0 ; i < (int)pVec.size() ; i++ )
		{
			if ( part == pVec[i] )
				currPartID = i;

			structureUI->structureBrowser->add( pVec[i]->GetName() );
		}

		if ( currPartID >= 0 && currPartID < (int)pVec.size() )
			structureUI->structureBrowser->select( currPartID+1 );

	}

	//==== FEA Export ====//
	sprintf( str, "%6.3f", structureMgrPtr->GetFinalTriArea() );
	structureUI->triAreaInput->value(str);

	structureUI->ExportNameInput->value( structureMgrPtr->GetExportFileName() );

	structureUI->ResultsDisplay->buffer()->text("");
	vector < Stringc > strVec = structureMgrPtr->GetFinalResultStringVec();
	for ( i = 0 ; i < (int)strVec.size() ; i++ )
	{
			structureUI->ResultsDisplay->buffer()->append( strVec[i] );
	}


	//==== Show/Hide UI Depending on Part ====//
	int type = -1;
	if ( part )
	{
		type = part->GetType();
	}

	if ( type != currPartType )
	{
		currPartType = type;
		structureUI->threePointGroup->hide();		
		structureUI->ribGroup->hide();		
		structureUI->sparGroup->hide();		

		if ( currPartType == Part::THREE_PNT_SLICE )
		{
			structureUI->threePointGroup->show();		
		}
		else if ( currPartType == Part::RIB )
		{
			structureUI->ribGroup->show();		
		}
		else if ( currPartType == Part::SPAR )
		{
			structureUI->sparGroup->show();		
		}


	}

}


void StructureScreen::show()
{
	update();

	structureUI->UIWindow->show();
	structureMgrPtr->SetActiveFlag( true );
}

void StructureScreen::screenCB( Fl_Widget* w )
{
	if ( w == structureUI->compChoice )
	{
		//==== Load List of Parts for Comp ====//
		int id = structureUI->compChoice->value();
		structureMgrPtr->setCurrGeomID( id );
	}
	else if ( w == structureUI->addStructureButton )
	{
		int type = structureUI->structureTypeChoice->value();
		if ( type >= 0 && type < Part::NUM_SLICE_TYPES )
			structureMgrPtr->CreateAddPart( type );
	}
	else if ( w == structureUI->deleteStructureButton )
	{
		structureMgrPtr->DeleteCurrPart();
	}
	else if ( w == structureUI->PartNameInput )
	{
		structureMgrPtr->GUI_Val( "PartName", Stringc(structureUI->PartNameInput->value()) );
	}
	else if ( w == structureUI->structureBrowser )
	{
		structureMgrPtr->GUI_Val( "PartID", structureUI->structureBrowser->value()-1 );
	}
	else if ( w == structureUI->ExportNameInput )
	{
		structureMgrPtr->GUI_Val( "ExportName", Stringc(structureUI->ExportNameInput->value()) );
	}
	
	else if ( w == structureUI->u1Slider )
		structureMgrPtr->GUI_Val( "U1", structureUI->u1Slider->value() );
	else if ( w == structureUI->w1Slider )
		structureMgrPtr->GUI_Val( "W1", structureUI->w1Slider->value() );
	else if ( w == structureUI->u1Input )
		structureMgrPtr->GUI_Val( "U1", atof( structureUI->u1Input->value() ) );
	else if ( w == structureUI->w1Input )
		structureMgrPtr->GUI_Val( "W1", atof( structureUI->w1Input->value() ) );

	else if ( w == structureUI->u2Slider )
		structureMgrPtr->GUI_Val( "U2", structureUI->u2Slider->value() );
	else if ( w == structureUI->w2Slider )
		structureMgrPtr->GUI_Val( "W2", structureUI->w2Slider->value() );
	else if ( w == structureUI->u2Input )
		structureMgrPtr->GUI_Val( "U2", atof( structureUI->u2Input->value() ) );
	else if ( w == structureUI->w2Input )
		structureMgrPtr->GUI_Val( "W2", atof( structureUI->w2Input->value() ) );

	else if ( w == structureUI->u3Slider )
		structureMgrPtr->GUI_Val( "U3", structureUI->u3Slider->value() );
	else if ( w == structureUI->w3Slider )
		structureMgrPtr->GUI_Val( "W3", structureUI->w3Slider->value() );
	else if ( w == structureUI->u3Input )
		structureMgrPtr->GUI_Val( "U3", atof( structureUI->u3Input->value() ) );
	else if ( w == structureUI->w3Input )
		structureMgrPtr->GUI_Val( "W3", atof( structureUI->w3Input->value() ) );

	else if ( w == structureUI->ribSpanSlider )
		structureMgrPtr->GUI_Val( "RibSpan", structureUI->ribSpanSlider->value() );
	else if ( w == structureUI->ribSpanInput )
		structureMgrPtr->GUI_Val( "RibSpan", atof( structureUI->ribSpanInput->value() ) );

	else if ( w == structureUI->sparChordSlider )
		structureMgrPtr->GUI_Val( "SparChord", structureUI->sparChordSlider->value() );
	else if ( w == structureUI->sparChordInput )
		structureMgrPtr->GUI_Val( "SparChord", atof( structureUI->sparChordInput->value() ) );

	else if ( w == structureUI->finalMeshStructureButton )
		structureMgrPtr->FinalMesh();
	else if ( w == structureUI->triAreaInput )
		structureMgrPtr->GUI_Val( "FinalTriArea", atof( structureUI->triAreaInput->value() ) );


	update();
}

