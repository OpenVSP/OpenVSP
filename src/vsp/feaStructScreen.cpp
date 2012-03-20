//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "feaStructScreen.h"
#include "groupScreen.h"
#include "scriptMgr.h"
#include "FeaMeshMgr.h"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FeaStructScreen::FeaStructScreen(ScreenMgr* mgr, Aircraft* airPtr)
{
	screenMgrPtr = mgr;
	aircraftPtr = airPtr;

	FEAStructUI* ui = feaStructUI = new FEAStructUI();
	feaStructUI->UIWindow->position( 760, 30 );

	ui->UIWindow->callback( staticCloseCB, this );

	m_DefEdgeSlider = new SliderInputCombo( ui->elemSizeSlider, ui->elemSizeInput );
	m_DefEdgeSlider->SetCallback( staticScreenCB, this );
	m_DefEdgeSlider->SetLimits( 0.0001, 10000.0 );
	m_DefEdgeSlider->SetRange( 0.5 );
	m_DefEdgeSlider->UpdateGui();

	m_MinSizeSlider = new SliderInputCombo( ui->minSizeSlider, ui->minSizeInput );
	m_MinSizeSlider->SetCallback( staticScreenCB, this );
	m_MinSizeSlider->SetLimits( 0.00001, 10000.0 );
	m_MinSizeSlider->SetRange( 0.5 );
	m_MinSizeSlider->UpdateGui();

	m_MaxGapSlider = new SliderInputCombo( ui->maxGapSlider, ui->maxGapInput );
	m_MaxGapSlider->SetCallback( staticScreenCB, this );
	m_MaxGapSlider->SetLimits( 0.0001, 10000.0 );
	m_MaxGapSlider->SetRange( 1.0 );
	m_MaxGapSlider->UpdateGui();

	m_NumCircSegSlider = new SliderInputCombo( ui->numCircSegSlider, ui->numCircSegInput );
	m_NumCircSegSlider->SetCallback( staticScreenCB, this );
	m_NumCircSegSlider->SetLimits( 3.0, 1000.0 );
	m_NumCircSegSlider->SetRange( 100.0 );
	m_NumCircSegSlider->UpdateGui();

	m_ThickScaleSlider = new SliderInputCombo( ui->thickScaleSlider, ui->thickScaleInput );
	m_ThickScaleSlider->SetCallback( staticScreenCB, this );
	m_ThickScaleSlider->SetLimits( 0.00001, 1000000.0 );
	m_ThickScaleSlider->SetRange( 0.5 );
	m_ThickScaleSlider->UpdateGui();

	ui->sectIDInput->callback( staticScreenCB, this );
	ui->sectIDLeftButton->callback( staticScreenCB, this );
	ui->sectIDRightButton->callback( staticScreenCB, this );

	ui->editTabs->callback( staticScreenCB, this );

	ui->sparIDInput->callback( staticScreenCB, this );
	ui->sparIDDownButton->callback( staticScreenCB, this );
	ui->sparIDUpButton->callback( staticScreenCB, this );
	ui->addSparButton->callback( staticScreenCB, this );
	ui->delSparButton->callback( staticScreenCB, this );


	m_SparThickSlider = new SliderInputCombo( ui->sparThickSlider, ui->sparThickInput );
	m_SparThickSlider->SetCallback( staticScreenCB, this );
	m_SparThickSlider->SetLimits( 0.00001, 1000000.0 );
	m_SparThickSlider->SetRange( 0.5 );
	m_SparThickSlider->UpdateGui();

	m_SparDensitySlider = new SliderInputCombo( ui->sparDensitySlider, ui->sparDensityInput );
	m_SparDensitySlider->SetCallback( staticScreenCB, this );
	m_SparDensitySlider->SetLimits( 0.00001, 1000000.0 );
	m_SparDensitySlider->SetRange( 1.0 );
	m_SparDensitySlider->UpdateGui();

	m_SparPosSlider = new SliderInputCombo( ui->sparPosSlider, ui->sparPosInput );
	m_SparPosSlider->SetCallback( staticScreenCB, this );
	m_SparPosSlider->SetLimits( 0, 1.0 );
	m_SparPosSlider->SetRange( 1.0 );
	m_SparPosSlider->UpdateGui();

	m_SparSweepSlider = new SliderInputCombo( ui->sparSweepSlider, ui->sparSweepInput );
	m_SparSweepSlider->SetCallback( staticScreenCB, this );
	m_SparSweepSlider->SetLimits( -89.0, 89.0 );
	m_SparSweepSlider->SetRange( 45 );
	m_SparSweepSlider->UpdateGui();

	ui->sparSweepAbsButton->callback( staticScreenCB, this );
	ui->sparSweepRelButton->callback( staticScreenCB, this );
	ui->trimSparButton->callback( staticScreenCB, this );

	ui->ribIDInput->callback( staticScreenCB, this );
	ui->ribIDDownButton->callback( staticScreenCB, this );
	ui->ribIDUpButton->callback( staticScreenCB, this );
	ui->addRibButton->callback( staticScreenCB, this );
	ui->delRibButton->callback( staticScreenCB, this );

	m_RibThickSlider = new SliderInputCombo( ui->ribThickSlider, ui->ribThickInput );
	m_RibThickSlider->SetCallback( staticScreenCB, this );
	m_RibThickSlider->SetLimits( 0.00001, 1000000.0 );
	m_RibThickSlider->SetRange( 0.5 );
	m_RibThickSlider->UpdateGui();

	m_RibDensitySlider = new SliderInputCombo( ui->ribDensitySlider, ui->ribDensityInput );
	m_RibDensitySlider->SetCallback( staticScreenCB, this );
	m_RibDensitySlider->SetLimits( 0.00001, 1000000.0 );
	m_RibDensitySlider->SetRange( 1.0 );
	m_RibDensitySlider->UpdateGui();

	m_RibPosSlider = new SliderInputCombo( ui->ribPosSlider, ui->ribPosInput );
	m_RibPosSlider->SetCallback( staticScreenCB, this );
	m_RibPosSlider->SetLimits( 0, 1.0 );
	m_RibPosSlider->SetRange( 1.0 );
	m_RibPosSlider->UpdateGui();

	m_RibSweepSlider = new SliderInputCombo( ui->ribSweepSlider, ui->ribSweepInput );
	m_RibSweepSlider->SetCallback( staticScreenCB, this );
	m_RibSweepSlider->SetLimits( -89.0, 89.0 );
	m_RibSweepSlider->SetRange( 45 );
	m_RibSweepSlider->UpdateGui();

	ui->ribSweepAbsButton->callback( staticScreenCB, this );
	ui->ribSweepRelButton->callback( staticScreenCB, this );
	ui->trimRibButton->callback( staticScreenCB, this );

	ui->exportFEAMeshButton->callback( staticScreenCB, this );
	ui->computeFEAMeshButton->callback( staticScreenCB, this );
	ui->drawMeshButton->callback( staticScreenCB, this );

	//==== Upper Skin ====//
	m_UpThickSlider = new SliderInputCombo( ui->upSkinThickSlider, ui->upSkinThickInput );
	m_UpThickSlider->SetCallback( staticScreenCB, this );
	m_UpThickSlider->SetLimits( 0.00001, 1000000.0 );
	m_UpThickSlider->SetRange( 0.5 );
	m_UpThickSlider->UpdateGui();

	m_UpDensitySlider = new SliderInputCombo( ui->upSkinDensitySlider, ui->upSkinDensityInput );
	m_UpDensitySlider->SetCallback( staticScreenCB, this );
	m_UpDensitySlider->SetLimits( 0.00001, 1000000.0 );
	m_UpDensitySlider->SetRange( 1.0 );
	m_UpDensitySlider->UpdateGui();

	ui->upSkinAddSpliceLineButton->callback( staticScreenCB, this );
	ui->upSkinDelSpliceLineButton->callback( staticScreenCB, this );
	ui->upSpliceLineIDDownButton->callback( staticScreenCB, this );
	ui->upSpliceLineIDUpButton->callback( staticScreenCB, this );
	ui->upSpliceLineIDInput->callback( staticScreenCB, this );

	m_UpSpliceLineLocSlider = new SliderInputCombo( ui->upSkinSpliceLineLocSlider, ui->upSkinSpliceLineLocInput );
	m_UpSpliceLineLocSlider->SetCallback( staticScreenCB, this );
	m_UpSpliceLineLocSlider->SetLimits( 0.0, 1.0 );
	m_UpSpliceLineLocSlider->SetRange( 1.0 );
	m_UpSpliceLineLocSlider->UpdateGui();

	m_UpDefThickSlider = new SliderInputCombo( ui->upSkinThickSlider, ui->upSkinThickInput );
	m_UpDefThickSlider->SetCallback( staticScreenCB, this );
	m_UpDefThickSlider->SetLimits( 0.00001, 1000000.0  );
	m_UpDefThickSlider->SetRange( 0.1 );
	m_UpDefThickSlider->UpdateGui();

	ui->upSkinExportNoButton->callback( staticScreenCB, this );
	ui->upSkinExportYesButton->callback( staticScreenCB, this );
	ui->upSkinAddSpliceLineButton->callback( staticScreenCB, this );
	ui->upSkinDelSpliceLineButton->callback( staticScreenCB, this );

	ui->upSkinAddSpliceButton->callback( staticScreenCB, this );
	ui->upSkinDelSpliceButton->callback( staticScreenCB, this );

	//==== Add Gl Window to Main Window ====//
	Fl_Widget* w = ui->upSkinSpliceLineGLGroup;
	ui->upSkinSpliceLineGLGroup->begin();
	m_UpSkinGLWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->upSkinSpliceLineGLGroup->end();

	m_UpSpliceLocSlider = new SliderInputCombo( ui->upSplicePosSlider, ui->upSplicePosInput  );
	m_UpSpliceLocSlider->SetCallback( staticScreenCB, this );
	m_UpSpliceLocSlider->SetLimits( 0.0, 1.0  );
	m_UpSpliceLocSlider->SetRange( 1.0 );

	m_UpSpliceThickSlider = new SliderInputCombo( ui->upSpliceThickSlider, ui->upSpliceThickInput );
	m_UpSpliceThickSlider->SetCallback( staticScreenCB, this );
	m_UpSpliceThickSlider->SetLimits( 0.00001, 1000000.0  );
	m_UpSpliceThickSlider->SetRange( 0.1 );

	ui->upSpliceIDLeftButton->callback( staticScreenCB, this );
	ui->upSpliceIDRightButton->callback( staticScreenCB, this );

	//==== Lower Skin ====//
	m_LowThickSlider = new SliderInputCombo( ui->lowSkinThickSlider, ui->lowSkinThickInput );
	m_LowThickSlider->SetCallback( staticScreenCB, this );
	m_LowThickSlider->SetLimits( 0.00001, 1000000.0 );
	m_LowThickSlider->SetRange( 0.1 );
	m_LowThickSlider->UpdateGui();

	m_LowDensitySlider = new SliderInputCombo( ui->lowSkinDensitySlider, ui->lowSkinDensityInput );
	m_LowDensitySlider->SetCallback( staticScreenCB, this );
	m_LowDensitySlider->SetLimits( 0.00001, 1000000.0 );
	m_LowDensitySlider->SetRange( 0.1 );
	m_LowDensitySlider->UpdateGui();

	ui->lowSkinAddSpliceLineButton->callback( staticScreenCB, this );
	ui->lowSkinDelSpliceLineButton->callback( staticScreenCB, this );
	ui->lowSpliceLineIDDownButton->callback( staticScreenCB, this );
	ui->lowSpliceLineIDUpButton->callback( staticScreenCB, this );
	ui->lowSpliceLineIDInput->callback( staticScreenCB, this );

	m_LowSpliceLineLocSlider = new SliderInputCombo( ui->lowSkinSpliceLineLocSlider, ui->lowSkinSpliceLineLocInput );
	m_LowSpliceLineLocSlider->SetCallback( staticScreenCB, this );
	m_LowSpliceLineLocSlider->SetLimits( 0.0, 1.0 );
	m_LowSpliceLineLocSlider->SetRange( 1.0 );
	m_LowSpliceLineLocSlider->UpdateGui();

	m_LowDefThickSlider = new SliderInputCombo( ui->lowSkinThickSlider, ui->lowSkinThickInput );
	m_LowDefThickSlider->SetCallback( staticScreenCB, this );
	m_LowDefThickSlider->SetLimits( 0.00001, 1000000.0  );
	m_LowDefThickSlider->SetRange( 0.1 );
	m_LowDefThickSlider->UpdateGui();

	ui->lowSkinExportNoButton->callback( staticScreenCB, this );
	ui->lowSkinExportYesButton->callback( staticScreenCB, this );
	ui->lowSkinAddSpliceLineButton->callback( staticScreenCB, this );
	ui->lowSkinDelSpliceLineButton->callback( staticScreenCB, this );

	ui->lowSkinAddSpliceButton->callback( staticScreenCB, this );
	ui->lowSkinDelSpliceButton->callback( staticScreenCB, this );

	//==== Add Gl Window to Main Window ====//
	w = ui->lowSkinSpliceLineGLGroup;
	ui->lowSkinSpliceLineGLGroup->begin();
	m_LowSkinGLWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h()); 
	ui->lowSkinSpliceLineGLGroup->end();

	m_LowSpliceLocSlider = new SliderInputCombo( ui->lowSplicePosSlider, ui->lowSplicePosInput  );
	m_LowSpliceLocSlider->SetCallback( staticScreenCB, this );
	m_LowSpliceLocSlider->SetLimits( 0.0, 1.0  );
	m_LowSpliceLocSlider->SetRange( 1.0 );

	m_LowSpliceThickSlider = new SliderInputCombo( ui->lowSpliceThickSlider, ui->lowSpliceThickInput );
	m_LowSpliceThickSlider->SetCallback( staticScreenCB, this );
	m_LowSpliceThickSlider->SetLimits( 0.00001, 1000000.0  );
	m_LowSpliceThickSlider->SetRange( 0.1 );

	ui->lowSpliceIDLeftButton->callback( staticScreenCB, this );
	ui->lowSpliceIDRightButton->callback( staticScreenCB, this );

	ui->outputText->buffer( &m_TextBuffer );

	//==== Point Mass =====//
	ui->ptMassIDInput->callback( staticScreenCB, this );
	ui->ptMassIDDownButton->callback( staticScreenCB, this );
	ui->ptMassIDUpButton->callback( staticScreenCB, this );
	ui->addPtMassButton->callback( staticScreenCB, this );
	ui->delPtMassButton->callback( staticScreenCB, this );

	m_pmXPosSlider = new SliderInputCombo( ui->pmXLocSlider, ui->pmXLocInput  );
	m_pmXPosSlider->SetCallback( staticScreenCB, this );
	m_pmXPosSlider->SetLimits( -10000.00, 10000.0  );
	m_pmXPosSlider->SetRange( 10.0 );

	m_pmYPosSlider = new SliderInputCombo( ui->pmYLocSlider, ui->pmYLocInput  );
	m_pmYPosSlider->SetCallback( staticScreenCB, this );
	m_pmYPosSlider->SetLimits( -10000.00, 10000.0  );
	m_pmYPosSlider->SetRange( 10.0 );

	m_pmZPosSlider = new SliderInputCombo( ui->pmZLocSlider, ui->pmZLocInput  );
	m_pmZPosSlider->SetCallback( staticScreenCB, this );
	m_pmZPosSlider->SetLimits( -10000.00, 10000.0  );
	m_pmZPosSlider->SetRange( 10.0 );

	ui->moveAttachPointButton->callback( staticScreenCB, this );

	ui->geomButton->callback( staticScreenCB, this );
	ui->massButton->callback( staticScreenCB, this );
	ui->nastranButton->callback( staticScreenCB, this );
	ui->thickButton->callback( staticScreenCB, this );
	ui->stlButton->callback( staticScreenCB, this );
	ui->geomToggle->value(1);
	ui->massToggle->value(1);
	ui->nastranToggle->value(1);
	ui->thickToggle->value(1);
	ui->stlToggle->value(0);
}

FeaStructScreen::~FeaStructScreen()
{
	delete m_DefEdgeSlider;
	delete m_MinSizeSlider;
	delete m_MaxGapSlider;
	delete m_NumCircSegSlider;
	delete m_ThickScaleSlider;

	delete m_RibThickSlider;
	delete m_RibPosSlider;
	delete m_RibSweepSlider;
	delete m_RibDensitySlider;

	delete m_SparThickSlider;
	delete m_SparPosSlider;
	delete m_SparSweepSlider;
	delete m_SparDensitySlider;

	delete m_UpThickSlider;
	delete m_UpDensitySlider;
	delete m_UpDefThickSlider;
	delete m_UpSpliceLineLocSlider;
	delete m_UpSpliceLocSlider;
	delete m_UpSpliceThickSlider;

	delete m_LowThickSlider;
	delete m_LowDensitySlider;
	delete m_LowDefThickSlider;
	delete m_LowSpliceLineLocSlider;
	delete m_LowSpliceLocSlider;
	delete m_LowSpliceThickSlider;

	delete m_pmXPosSlider;
	delete m_pmYPosSlider;
	delete m_pmZPosSlider;

	delete m_UpSkinGLWin;
	delete m_LowSkinGLWin;

	delete feaStructUI;
}

void FeaStructScreen::closeCB( Fl_Widget* w )
{
	feaStructUI->drawMeshButton->value(0);
	feaMeshMgrPtr->SetDrawMeshFlag( false );
	feaMeshMgrPtr->SetDrawFlag( false );

	feaMeshMgrPtr->SaveData();

	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->setNoShowFlag(0);
	}
	screenMgrPtr->update( GEOM_SCREEN );

	feaStructUI->UIWindow->hide();
	aircraftPtr->triggerDraw();
}

void FeaStructScreen::update()
{
	char str[256];

	//==== Default Elem Size ====//
	m_DefEdgeSlider->SetVal( feaMeshMgrPtr->GetDefElemSize() );
	m_DefEdgeSlider->UpdateGui();

	m_MinSizeSlider->SetVal( feaMeshMgrPtr->GetGridDensityPtr()->GetMinLen() );
	m_MinSizeSlider->UpdateGui();

	m_MaxGapSlider->SetVal( feaMeshMgrPtr->GetGridDensityPtr()->GetMaxGap() );
	m_MaxGapSlider->UpdateGui();

	m_NumCircSegSlider->SetVal( feaMeshMgrPtr->GetGridDensityPtr()->GetNCircSeg() );
	m_NumCircSegSlider->UpdateGui();

	m_ThickScaleSlider->SetVal( feaMeshMgrPtr->GetThickScale() );
	m_ThickScaleSlider->UpdateGui();

	//==== SectID ====//
	sprintf( str, "        %d ", feaMeshMgrPtr->GetCurrSectID() );
	feaStructUI->sectIDInput->value(str);

	//==== Spar Rib ID ====//
	sprintf( str, "        %d ", feaMeshMgrPtr->GetCurrSparID() );
	if ( feaMeshMgrPtr->GetNumSpars() == 0 )		
		sprintf( str, " " );
	feaStructUI->sparIDInput->value(str);

	sprintf( str, "        %d ", feaMeshMgrPtr->GetCurrRibID() );
	if ( feaMeshMgrPtr->GetNumRibs() == 0 )		
		sprintf( str, " " );
	feaStructUI->ribIDInput->value(str);

	FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
	if ( rib )
	{
		feaStructUI->ribEditGroup->show();
		m_RibThickSlider->SetVal( rib->m_Thick );
		m_RibThickSlider->UpdateGui();

		m_RibDensitySlider->SetVal( rib->GetDensity() );
		m_RibDensitySlider->UpdateGui();

		m_RibPosSlider->SetVal( rib->m_PerSpan );
		m_RibPosSlider->UpdateGui();

		m_RibSweepSlider->SetVal( rib->m_Sweep );
		m_RibSweepSlider->UpdateGui();

		if ( rib->m_AbsSweepFlag )
		{
			feaStructUI->ribSweepAbsButton->value(1);
			feaStructUI->ribSweepRelButton->value(0);
		}
		else
		{
			feaStructUI->ribSweepAbsButton->value(0);
			feaStructUI->ribSweepRelButton->value(1);
		}
		if ( rib->m_TrimFlag )
			feaStructUI->trimRibButton->value(1);
		else
			feaStructUI->trimRibButton->value(0);
	}
	else
	{
		feaStructUI->ribEditGroup->hide();
	}


	FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
	if ( spar )
	{
		feaStructUI->sparEditGroup->show();
		m_SparThickSlider->SetVal( spar->m_Thick );
		m_SparThickSlider->UpdateGui();

		m_SparDensitySlider->SetVal( spar->GetDensity() );
		m_SparDensitySlider->UpdateGui();

		m_SparPosSlider->SetVal( spar->m_PerChord );
		m_SparPosSlider->UpdateGui();

		m_SparSweepSlider->SetVal( spar->m_Sweep );
		m_SparSweepSlider->UpdateGui();

		if ( spar->m_AbsSweepFlag )
		{
			feaStructUI->sparSweepAbsButton->value(1);
			feaStructUI->sparSweepRelButton->value(0);
		}
		else
		{
			feaStructUI->sparSweepAbsButton->value(0);
			feaStructUI->sparSweepRelButton->value(1);
		}
		if ( spar->m_TrimFlag )
			feaStructUI->trimSparButton->value(1);
		else
			feaStructUI->trimSparButton->value(0);
	}
	else
	{
		feaStructUI->sparEditGroup->hide();
	}

	//===== Upper Skin ====//
	FeaSkin* upskin = feaMeshMgrPtr->GetCurrUpperSkin();
	if ( upskin && upskin->GetExportFlag() )
	{
		feaStructUI->upperSkinEditGroup->show();
		feaStructUI->upSkinExportNoButton->value(0);
		feaStructUI->upSkinExportYesButton->value(1);

		m_UpDefThickSlider->SetVal( upskin->GetDefaultThick() );
		m_UpDefThickSlider->UpdateGui();

		m_UpDensitySlider->SetVal( upskin->GetDensity() );
		m_UpDensitySlider->UpdateGui();

		FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
		if ( sl )
		{
			m_UpSpliceLineLocSlider->SetVal( sl->m_PerSpan );
			m_UpSpliceLineLocSlider->UpdateGui();

			FeaSplice* splice = sl->GetEditSplice();
			if ( splice )
			{
				m_UpSpliceLocSlider->SetVal( splice->m_Pos );
				m_UpSpliceLocSlider->UpdateGui();
				m_UpSpliceThickSlider->SetVal( splice->m_Thick );
				m_UpSpliceThickSlider->UpdateGui();
			}
		}
	
		//==== Splice Line ID ====//
		sprintf( str, "        %d ", upskin->GetCurrSpliceLineID() );
		if ( (int)upskin->m_SpliceLineVec.size() == 0 )		
			sprintf( str, " " );
		feaStructUI->upSpliceLineIDInput->value(str);
	}
	else
	{
		feaStructUI->upperSkinEditGroup->hide();
		feaStructUI->upSkinExportNoButton->value(1);
		feaStructUI->upSkinExportYesButton->value(0);
		feaStructUI->upSpliceLineIDInput->value("");
	}

	m_UpSkinGLWin->setDrawBase( NULL );
	if ( upskin )
	{
		FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
		if ( sl )
		{
			m_UpSkinGLWin->setDrawBase( sl );

			if ( sl->GetMode() == FeaSpliceLine::ADD_MODE )
				feaStructUI->upSkinAddSpliceButton->value(1);
			else
				feaStructUI->upSkinAddSpliceButton->value(0);
				
			if ( sl->GetMode() == FeaSpliceLine::DEL_MODE )
				feaStructUI->upSkinDelSpliceButton->value(1);
			else
				feaStructUI->upSkinDelSpliceButton->value(0);
		}
	}
	m_UpSkinGLWin->redraw();

	//===== Lower Skin ====//
	FeaSkin* lowskin = feaMeshMgrPtr->GetCurrLowerSkin();
	if ( lowskin && lowskin->GetExportFlag() )
	{
		feaStructUI->lowerSkinEditGroup->show();
		feaStructUI->lowSkinExportNoButton->value(0);
		feaStructUI->lowSkinExportYesButton->value(1);

		m_LowDefThickSlider->SetVal( lowskin->GetDefaultThick() );
		m_LowDefThickSlider->UpdateGui();

		m_LowDensitySlider->SetVal( lowskin->GetDensity() );
		m_LowDensitySlider->UpdateGui();

		FeaSpliceLine* sl = lowskin->GetCurrSpliceLine();
		if ( sl )
		{
			m_LowSpliceLineLocSlider->SetVal( sl->m_PerSpan );
			m_LowSpliceLineLocSlider->UpdateGui();

			FeaSplice* splice = sl->GetEditSplice();
			if ( splice )
			{
				m_LowSpliceLocSlider->SetVal( splice->m_Pos );
				m_LowSpliceLocSlider->UpdateGui();
				m_LowSpliceThickSlider->SetVal( splice->m_Thick );
				m_LowSpliceThickSlider->UpdateGui();
			}
		}
	
		//==== Splice Line ID ====//
		sprintf( str, "        %d ", lowskin->GetCurrSpliceLineID() );
		if ( (int)lowskin->m_SpliceLineVec.size() == 0 )		
			sprintf( str, " " );
		feaStructUI->lowSpliceLineIDInput->value(str);
	}
	else
	{
		feaStructUI->lowerSkinEditGroup->hide();
		feaStructUI->lowSkinExportNoButton->value(1);
		feaStructUI->lowSkinExportYesButton->value(0);
		feaStructUI->lowSpliceLineIDInput->value("");
	}

	m_LowSkinGLWin->setDrawBase( NULL );
	if ( lowskin )
	{
		FeaSpliceLine* sl = lowskin->GetCurrSpliceLine();
		if ( sl )
		{
			m_LowSkinGLWin->setDrawBase( sl );

			if ( sl->GetMode() == FeaSpliceLine::ADD_MODE )
				feaStructUI->lowSkinAddSpliceButton->value(1);
			else
				feaStructUI->lowSkinAddSpliceButton->value(0);
				
			if ( sl->GetMode() == FeaSpliceLine::DEL_MODE )
				feaStructUI->lowSkinDelSpliceButton->value(1);
			else
				feaStructUI->lowSkinDelSpliceButton->value(0);
		}
	}
	m_LowSkinGLWin->redraw();


	//==== Point Mass ====//
	FeaPointMass* pmass = feaMeshMgrPtr->GetCurrPointMass();
	if ( pmass )
	{
		feaStructUI->ptMassEditGroup->show();
		m_pmXPosSlider->SetVal( pmass->m_Pos.x() );
		m_pmYPosSlider->SetVal( pmass->m_Pos.y() );
		m_pmZPosSlider->SetVal( pmass->m_Pos.z() );
		m_pmXPosSlider->UpdateGui();
		m_pmYPosSlider->UpdateGui();
		m_pmZPosSlider->UpdateGui();

		//==== Point Mass ID ====//
		sprintf( str, "        %d ", feaMeshMgrPtr->GetCurrPointMassID() );
		if ( feaMeshMgrPtr->GetNumPointMasses() == 0 )		
			sprintf( str, " " );
		feaStructUI->ptMassIDInput->value(str);

		sprintf( str, "%f", pmass->m_AttachPos.x() );
		feaStructUI->XAttachInput->value( str );
		sprintf( str, "%f", pmass->m_AttachPos.y() );
		feaStructUI->YAttachInput->value( str );
		sprintf( str, "%f", pmass->m_AttachPos.z() );
		feaStructUI->ZAttachInput->value( str );
	}
	else
	{
		feaStructUI->ptMassEditGroup->hide();
		feaStructUI->ptMassIDInput->value("");
	}

	if ( feaMeshMgrPtr->GetDrawAttachPointsFlag() )
		feaStructUI->moveAttachPointButton->value( 1 );
	else
		feaStructUI->moveAttachPointButton->value( 0 );

	Stringc massname = feaMeshMgrPtr->GetFeaExportFileName( FeaMeshMgr::MASS_FILE_NAME );
	feaStructUI->massName->value( truncateFileName(massname, 40 ) );
	Stringc nastranname = feaMeshMgrPtr->GetFeaExportFileName( FeaMeshMgr::NASTRAN_FILE_NAME );
	feaStructUI->nastranName->value( truncateFileName(nastranname, 40 ) );
	Stringc geomname = feaMeshMgrPtr->GetFeaExportFileName( FeaMeshMgr::GEOM_FILE_NAME );
	feaStructUI->geomName->value( truncateFileName(geomname, 40 ) );
	Stringc thickname = feaMeshMgrPtr->GetFeaExportFileName( FeaMeshMgr::THICK_FILE_NAME );
	feaStructUI->thickName->value( truncateFileName(thickname, 40 ) );
	Stringc stlname = feaMeshMgrPtr->GetFeaExportFileName( FeaMeshMgr::STL_FEA_NAME );
	feaStructUI->stlName->value( truncateFileName(stlname, 40 ) );

}

Stringc FeaStructScreen::truncateFileName( const char* fn, int len )
{
	Stringc trunc( fn );
	if ( trunc.get_length() > len )
	{
		trunc.delete_range( 0, trunc.get_length()-len );
		trunc.overwrite_at_position(0, "...");
	}
	return trunc;
}

void FeaStructScreen::setMeshExportFlags()
{
	feaMeshMgrPtr->SetExportFileFlag( !!feaStructUI->massToggle->value(), FeaMeshMgr::MASS_FILE_NAME );
	feaMeshMgrPtr->SetExportFileFlag( !!feaStructUI->nastranToggle->value(), FeaMeshMgr::NASTRAN_FILE_NAME );
	feaMeshMgrPtr->SetExportFileFlag( !!feaStructUI->geomToggle->value(), FeaMeshMgr::GEOM_FILE_NAME );
	feaMeshMgrPtr->SetExportFileFlag( !!feaStructUI->thickToggle->value(), FeaMeshMgr::THICK_FILE_NAME );
	feaMeshMgrPtr->SetExportFileFlag( !!feaStructUI->stlToggle->value(), FeaMeshMgr::STL_FEA_NAME );
}

void FeaStructScreen::show()
{
	feaMeshMgrPtr->SetDrawFlag( true );

	feaMeshMgrPtr->LoadSurfaces();
	update();

	feaStructUI->UIWindow->show();

	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->setNoShowFlag(1);
	}
	screenMgrPtr->update( GEOM_SCREEN );
	aircraftPtr->triggerDraw();
}

void FeaStructScreen::addOutputText( const char* text )
{
	m_TextBuffer.append( text );
	feaStructUI->outputText->move_down();
	feaStructUI->outputText->show_insert_position();
	Fl::flush();
}

void FeaStructScreen::screenCB( Fl_Widget* w )
{
	if ( m_DefEdgeSlider->GuiChanged( w ) )
	{
		feaMeshMgrPtr->SetDefElemSize( m_DefEdgeSlider->GetVal() );
	}
	if ( m_MinSizeSlider->GuiChanged( w ) )
	{
		feaMeshMgrPtr->GetGridDensityPtr()->SetMinLen( m_MinSizeSlider->GetVal() );
	}
	if ( m_MaxGapSlider->GuiChanged( w ) )
	{
		feaMeshMgrPtr->GetGridDensityPtr()->SetMaxGap( m_MaxGapSlider->GetVal() );
	}
	if ( m_NumCircSegSlider->GuiChanged( w ) )
	{
		feaMeshMgrPtr->GetGridDensityPtr()->SetNCircSeg( m_NumCircSegSlider->GetVal() );
	}
	else if ( m_ThickScaleSlider->GuiChanged( w ) )
	{
		feaMeshMgrPtr->SetThickScale( m_ThickScaleSlider->GetVal() );
	}
	else if ( w == feaStructUI->sectIDInput )		// Sect ID
	{
		int id = atoi( feaStructUI->sectIDInput->value() );
		feaMeshMgrPtr->SetCurrSectID( id );
		update();
	}
	else if ( w == feaStructUI->sectIDLeftButton )
	{
		int id = feaMeshMgrPtr->GetCurrSectID() - 1;
		feaMeshMgrPtr->SetCurrSectID( id );
		update();
	}
	else if ( w == feaStructUI->sectIDRightButton )
	{
		int id = feaMeshMgrPtr->GetCurrSectID() + 1;
		feaMeshMgrPtr->SetCurrSectID( id );
		update();
	}
	else if ( w == feaStructUI->sparIDInput )		// Spar ID
	{
		int id = atoi( feaStructUI->sparIDInput->value() );
		feaMeshMgrPtr->SetCurrSparID( id );
		update();
	}
	else if ( w == feaStructUI->sparIDDownButton )
	{
		int id = feaMeshMgrPtr->GetCurrSparID() - 1;
		feaMeshMgrPtr->SetCurrSparID( id );
		update();
	}
	else if ( w == feaStructUI->sparIDUpButton )
	{
		int id = feaMeshMgrPtr->GetCurrSparID() + 1;
		feaMeshMgrPtr->SetCurrSparID( id );
		update();
	}
	else if ( w == feaStructUI->ribIDInput )		// Rib ID
	{
		int id = atoi( feaStructUI->ribIDInput->value() );
		feaMeshMgrPtr->SetCurrRibID( id );
		update();
	}
	else if ( w == feaStructUI->ribIDDownButton )
	{
		int id = feaMeshMgrPtr->GetCurrRibID() - 1;
		feaMeshMgrPtr->SetCurrRibID( id );
		update();
	}
	else if ( w == feaStructUI->ribIDUpButton )
	{
		int id = feaMeshMgrPtr->GetCurrRibID() + 1;
		feaMeshMgrPtr->SetCurrRibID( id );
		update();
	}
	else if ( w == feaStructUI->addRibButton )		// Add Ribs
	{
		feaMeshMgrPtr->AddRib();
		update();
	}
	else if ( w == feaStructUI->delRibButton )
	{
		feaMeshMgrPtr->DelCurrRib();
	}
	else if ( m_RibThickSlider->GuiChanged( w ) )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->m_Thick = m_RibThickSlider->GetVal();
		}
	}
	else if ( m_RibDensitySlider->GuiChanged( w ) )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->SetDensity( m_RibDensitySlider->GetVal() );
		}
	}
	else if ( m_RibPosSlider->GuiChanged( w ) )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->m_PerSpan = m_RibPosSlider->GetVal();
			rib->ComputeEndPoints();
		}
	}
	else if ( m_RibSweepSlider->GuiChanged( w ) )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->m_Sweep = m_RibSweepSlider->GetVal();
			rib->ComputeEndPoints();
		}
	}
	else if ( w == feaStructUI->ribSweepAbsButton )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->m_AbsSweepFlag = true;
			rib->ComputeEndPoints();
			update();
		}
	}
	else if ( w == feaStructUI->ribSweepRelButton )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			rib->m_AbsSweepFlag = false;
			rib->ComputeEndPoints();
			update();
		}
	}
	else if ( w == feaStructUI->trimRibButton )
	{
		FeaRib* rib = feaMeshMgrPtr->GetCurrRib();
		if ( rib )
		{
			if ( feaStructUI->trimRibButton->value() )
				rib->m_TrimFlag = true;
			else
				rib->m_TrimFlag = false;
			rib->ComputeEndPoints();
		}
	}
	else if ( m_SparThickSlider->GuiChanged( w ) )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->m_Thick = m_SparThickSlider->GetVal();
		}
	}
	else if ( m_SparDensitySlider->GuiChanged( w ) )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->SetDensity( m_SparDensitySlider->GetVal() );
		}
	}
	else if ( m_SparPosSlider->GuiChanged( w ) )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->m_PerChord = m_SparPosSlider->GetVal();
			spar->ComputeEndPoints();
		}
	}
	else if ( m_SparSweepSlider->GuiChanged( w ) )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->m_Sweep = m_SparSweepSlider->GetVal();
			spar->ComputeEndPoints();
		}
	}
	else if ( w == feaStructUI->sparSweepAbsButton )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->m_AbsSweepFlag = true;
			spar->ComputeEndPoints();
			update();
		}
	}
	else if ( w == feaStructUI->sparSweepRelButton )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			spar->m_AbsSweepFlag = false;
			spar->ComputeEndPoints();
			update();
		}
	}
	else if ( w == feaStructUI->trimSparButton )
	{
		FeaSpar* spar = feaMeshMgrPtr->GetCurrSpar();
		if ( spar )
		{
			if ( feaStructUI->trimSparButton->value() )
				spar->m_TrimFlag = true;
			else
				spar->m_TrimFlag = false;
			spar->ComputeEndPoints();
		}
	}
	else if ( w == feaStructUI->addSparButton )			
	{
		feaMeshMgrPtr->AddSpar();
		update();
	}
	else if ( w == feaStructUI->delSparButton )
	{
		feaMeshMgrPtr->DelCurrSpar();
	}
	else if ( w == feaStructUI->exportFEAMeshButton )
	{
		feaMeshMgrPtr->Build();
		feaMeshMgrPtr->Export();
		feaMeshMgrPtr->SetDrawMeshFlag( true );
		feaStructUI->drawMeshButton->value(1);
	}
	else if ( w == feaStructUI->computeFEAMeshButton )
	{
		feaMeshMgrPtr->Build();
		feaMeshMgrPtr->SetDrawMeshFlag( true );
		feaStructUI->drawMeshButton->value(1);
	}
	else if ( w == feaStructUI->drawMeshButton )
	{
		if ( feaStructUI->drawMeshButton->value() )
			feaMeshMgrPtr->SetDrawMeshFlag( true );
		else
			feaMeshMgrPtr->SetDrawMeshFlag( false );
	}
	else if ( w == feaStructUI->editTabs )
	{
		Fl_Widget* tw = feaStructUI->editTabs->value();
		if ( tw == feaStructUI->upperSkinGroup )
			feaMeshMgrPtr->SetCurrEditType( FeaMeshMgr::UP_SKIN_EDIT );
		else if ( tw == feaStructUI->lowerSkinGroup  )
			feaMeshMgrPtr->SetCurrEditType( FeaMeshMgr::LOW_SKIN_EDIT );
		else if ( tw == feaStructUI->ribGroup  )
			feaMeshMgrPtr->SetCurrEditType( FeaMeshMgr::RIB_EDIT );
		else if ( tw == feaStructUI->sparGroup  )
			feaMeshMgrPtr->SetCurrEditType( FeaMeshMgr::SPAR_EDIT );
		else if ( tw == feaStructUI->pointMassGroup  )
			feaMeshMgrPtr->SetCurrEditType( FeaMeshMgr::POINT_MASS_EDIT );

		update();
	}
	//==== Upper Skin ====//
	else if ( w == feaStructUI->upSkinExportNoButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			skin->SetExportFlag( false );
		}
		update();
	}
	else if ( w == feaStructUI->upSkinExportYesButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			skin->SetExportFlag( true );
		}
		update();
	}
	else if ( w == feaStructUI->upSkinAddSpliceLineButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			skin->AddSpliceLine();
			skin->ComputeSpliceLineEndPoints();
		}
		update();
	}
	else if ( w == feaStructUI->upSkinDelSpliceLineButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			skin->DelCurrSpliceLine();
		}
		update();
	}
	else if ( w == feaStructUI->upSpliceLineIDInput )		// Splice Line ID
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			int id = atoi( feaStructUI->upSpliceLineIDInput->value() );
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( w == feaStructUI->upSpliceLineIDDownButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			int id = skin->GetCurrSpliceLineID() - 1;
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( w == feaStructUI->upSpliceLineIDUpButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( skin )
		{
			int id = skin->GetCurrSpliceLineID() + 1;
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( m_UpSpliceLineLocSlider->GuiChanged( w ) )
	{
		FeaSkin*  upskin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( upskin )
		{
			FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
			if ( sl )
			{
				sl->m_PerSpan = m_UpSpliceLineLocSlider->GetVal();
				upskin->ComputeSpliceLineEndPoints();
			}
		}
	}
	else if ( m_UpDefThickSlider->GuiChanged( w ) )
	{
		FeaSkin*  upskin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( upskin )
		{
			upskin->SetDefaultThick( m_UpDefThickSlider->GetVal() );
		}
	}
	else if ( m_UpDensitySlider->GuiChanged( w ) )
	{
		FeaSkin*  upskin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( upskin )
		{
			upskin->SetDensity( m_UpDensitySlider->GetVal() );
		}
	}
	else if ( w == feaStructUI->upSkinAddSpliceButton )
	{
		FeaSkin*  upskin = feaMeshMgrPtr->GetCurrUpperSkin();
		if ( upskin )
		{
			FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
			if ( sl )
			{
				sl->SetMode( FeaSpliceLine::ADD_MODE );
			}
		}
	}
	else if ( w == feaStructUI->upSkinDelSpliceButton )
	{
		if ( feaMeshMgrPtr->GetCurrUpperSkin() )
			if ( feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->DelEditSplice();
				update();
			}
	}
	else if ( m_UpSpliceLocSlider->GuiChanged( w ) )
	{
		if ( feaMeshMgrPtr->GetCurrUpperSkin() )
			if ( feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine() )
			{
				double xc = m_UpSpliceLocSlider->GetVal();
				feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->SetEditSpliceLoc( xc ); 
				FeaSplice* splice = feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->GetEditSplice();
				m_UpSkinGLWin->redraw();
			}
	}
	else if ( m_UpSpliceThickSlider->GuiChanged( w ) )
	{
		if ( feaMeshMgrPtr->GetCurrUpperSkin() )
			if ( feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine() )
			{
				FeaSplice* splice = feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->GetEditSplice();
				if ( splice )
				{
					splice->m_Thick = m_UpSpliceThickSlider->GetVal();
					m_UpSkinGLWin->redraw();
				}
			}
	}
	else if ( w == feaStructUI->upSpliceIDLeftButton )
	{
		if ( feaMeshMgrPtr->GetCurrUpperSkin() )
			if ( feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->ChangeEditSplice( -1 );
				update();
			}
	}
	else if ( w == feaStructUI->upSpliceIDRightButton )
	{
		if ( feaMeshMgrPtr->GetCurrUpperSkin() )
			if ( feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrUpperSkin()->GetCurrSpliceLine()->ChangeEditSplice( 1 );
				update();
			}
	}
	//==== Lower Skin ====//
	else if ( w == feaStructUI->lowSkinExportNoButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
			skin->SetExportFlag( false );
		update();
	}
	else if ( w == feaStructUI->lowSkinExportYesButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
			skin->SetExportFlag( true );
		update();
	}
	else if ( w == feaStructUI->lowSkinAddSpliceLineButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			skin->AddSpliceLine();
			skin->ComputeSpliceLineEndPoints();
		}
		update();
	}
	else if ( w == feaStructUI->lowSkinDelSpliceLineButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
			skin->DelCurrSpliceLine();
		update();
	}
	else if ( w == feaStructUI->lowSpliceLineIDInput )		// Splice Line ID
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			int id = atoi( feaStructUI->lowSpliceLineIDInput->value() );
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( w == feaStructUI->lowSpliceLineIDDownButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			int id = skin->GetCurrSpliceLineID() - 1;
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( w == feaStructUI->lowSpliceLineIDUpButton )
	{
		FeaSkin* skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			int id = skin->GetCurrSpliceLineID() + 1;
			skin->SetCurrSpliceLineID( id );
			update();
		}
	}
	else if ( m_LowSpliceLineLocSlider->GuiChanged( w ) )
	{
		FeaSkin*  skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			FeaSpliceLine* sl = skin->GetCurrSpliceLine();
			if ( sl )
			{
				sl->m_PerSpan = m_LowSpliceLineLocSlider->GetVal();
				skin->ComputeSpliceLineEndPoints();
			}
		}
	}
	else if ( m_LowDefThickSlider->GuiChanged( w ) )
	{
		FeaSkin*  skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
			skin->SetDefaultThick( m_LowDefThickSlider->GetVal() );
	}
	else if ( m_LowDensitySlider->GuiChanged( w ) )
	{
		FeaSkin*  skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
			skin->SetDensity( m_LowDensitySlider->GetVal() );
	}
	else if ( w == feaStructUI->lowSkinAddSpliceButton )
	{
		FeaSkin*  skin = feaMeshMgrPtr->GetCurrLowerSkin();
		if ( skin )
		{
			FeaSpliceLine* sl = skin->GetCurrSpliceLine();
			if ( sl )
			{
				sl->SetMode( FeaSpliceLine::ADD_MODE );
			}
		}
	}
	else if ( w == feaStructUI->lowSkinDelSpliceButton )
	{
		if ( feaMeshMgrPtr->GetCurrLowerSkin() )
			if ( feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->DelEditSplice();
				update();
			}
	}
	else if ( m_LowSpliceLocSlider->GuiChanged( w ) )
	{
		if ( feaMeshMgrPtr->GetCurrLowerSkin() )
			if ( feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine() )
			{
				double xc = m_LowSpliceLocSlider->GetVal();
				feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->SetEditSpliceLoc( xc ); 
				FeaSplice* splice = feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->GetEditSplice();
				m_LowSkinGLWin->redraw();
			}
	}
	else if ( m_LowSpliceThickSlider->GuiChanged( w ) )
	{
		if ( feaMeshMgrPtr->GetCurrLowerSkin() )
			if ( feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine() )
			{
				FeaSplice* splice = feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->GetEditSplice();
				if ( splice )
				{
					splice->m_Thick = m_LowSpliceThickSlider->GetVal();
					m_LowSkinGLWin->redraw();
				}
			}
	}
	else if ( w == feaStructUI->lowSpliceIDLeftButton )
	{
		if ( feaMeshMgrPtr->GetCurrLowerSkin() )
			if ( feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->ChangeEditSplice( -1 );
				update();
			}
	}
	else if ( w == feaStructUI->lowSpliceIDRightButton )
	{
		if ( feaMeshMgrPtr->GetCurrLowerSkin() )
			if ( feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine() )
			{
				feaMeshMgrPtr->GetCurrLowerSkin()->GetCurrSpliceLine()->ChangeEditSplice( 1 );
				update();
			}
	}
	else if ( w == feaStructUI->moveAttachPointButton )
	{
		int val = feaStructUI->moveAttachPointButton->value();
		feaMeshMgrPtr->SetDrawAttachPointsFlag( !!val );
		update();
	}
	else if ( w == feaStructUI->addPtMassButton )			
	{
		feaMeshMgrPtr->AddPointMass();
		update();
	}
	else if ( w == feaStructUI->delPtMassButton )			
	{
		feaMeshMgrPtr->DelCurrPointMass();
		update();
	}
	else if ( w == feaStructUI->ptMassIDInput )			
	{
		int id = atoi( feaStructUI->ptMassIDInput->value() );
		feaMeshMgrPtr->SetCurrPointMassID( id );
		update();
	}
	else if ( w == feaStructUI->ptMassIDUpButton )			
	{
		int id = feaMeshMgrPtr->GetCurrPointMassID() + 1;
		feaMeshMgrPtr->SetCurrPointMassID( id );
		update();
	}
	else if ( w == feaStructUI->ptMassIDDownButton )			
	{
		int id = feaMeshMgrPtr->GetCurrPointMassID() - 1;
		feaMeshMgrPtr->SetCurrPointMassID( id );
		update();
	}
	else if ( w == feaStructUI->massButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select Mass .dat file.", "*.dat");
		if ( newfile != NULL )
			feaMeshMgrPtr->SetFeaExportFileName( newfile, FeaMeshMgr::MASS_FILE_NAME );
		update();
	}
	else if ( w == feaStructUI->nastranButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select NASTRAN .dat file.", "*.dat");
		if ( newfile != NULL )
			feaMeshMgrPtr->SetFeaExportFileName( newfile, FeaMeshMgr::NASTRAN_FILE_NAME );
		update();
	}
	else if ( w == feaStructUI->geomButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select Calculix Geom .dat file.", "*.dat");
		if ( newfile != NULL )
			feaMeshMgrPtr->SetFeaExportFileName( newfile, FeaMeshMgr::GEOM_FILE_NAME );
		update();
	}
	else if ( w == feaStructUI->thickButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select Calculix Thickness .dat file.", "*.dat");
		if ( newfile != NULL )
			feaMeshMgrPtr->SetFeaExportFileName( newfile, FeaMeshMgr::THICK_FILE_NAME );
		update();
	}
	else if ( w == feaStructUI->stlButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .stl file.", "*.stl");
		if ( newfile != NULL )
			feaMeshMgrPtr->SetFeaExportFileName( newfile, FeaMeshMgr::STL_FEA_NAME );
		update();
	}
	else if ( m_pmXPosSlider->GuiChanged( w ) )
	{
		FeaPointMass* pm = feaMeshMgrPtr->GetCurrPointMass();
		if ( pm )
			pm->m_Pos.set_x(  m_pmXPosSlider->GetVal() );
	}
	else if ( m_pmYPosSlider->GuiChanged( w ) )
	{
		FeaPointMass* pm = feaMeshMgrPtr->GetCurrPointMass();
		if ( pm )
			pm->m_Pos.set_y(  m_pmYPosSlider->GetVal() );
	}
	else if ( m_pmZPosSlider->GuiChanged( w ) )
	{
		FeaPointMass* pm = feaMeshMgrPtr->GetCurrPointMass();
		if ( pm )
			pm->m_Pos.set_z(  m_pmZPosSlider->GetVal() );
	}

	aircraftPtr->triggerDraw();

}

