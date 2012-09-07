//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "parmLinkScreen.h"
#include "parmLinkMgr.h"
#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmLinkScreen::ParmLinkScreen(ScreenMgr* mgr, Aircraft* airPtr) : VspScreen( mgr )
{
	aircraftPtr = airPtr;

	ParmLinkUI* ui = parmLinkUI = new ParmLinkUI();
	parmLinkUI->UIWindow->position( 780, 30 );

	ui->compAChoice->callback( staticScreenCB, this );
	ui->groupAChoice->callback( staticScreenCB, this );
	ui->parmAChoice->callback( staticScreenCB, this );
	ui->compBChoice->callback( staticScreenCB, this );
	ui->groupBChoice->callback( staticScreenCB, this );
	ui->parmBChoice->callback( staticScreenCB, this );

	ui->offsetButton->callback( staticScreenCB, this );
	ui->scaleButton->callback( staticScreenCB, this );
	ui->lowerLimitButton->callback( staticScreenCB, this );
	ui->upperLimitButton->callback( staticScreenCB, this );
	ui->addLinkButton->callback( staticScreenCB, this );
	ui->deleteLinkButton->callback( staticScreenCB, this );
	ui->deleteAllLinksButton->callback( staticScreenCB, this );
	ui->linkBrowser->callback( staticScreenCB, this );

	ui->linkAllCompButton->callback( staticScreenCB, this );
	ui->linkAllGroupButton->callback( staticScreenCB, this );


	m_OffsetSlider = new SliderInputCombo( ui->offsetSlider, ui->offsetInput );
	m_OffsetSlider->SetCallback( staticScreenCB, this );
	m_OffsetSlider->SetLimits( -1000000.0, 1000000.0 );
	m_OffsetSlider->SetRange( 100 );
	m_OffsetSlider->UpdateGui();

	m_ScaleSlider = new SliderInputCombo( ui->scaleSlider, ui->scaleInput );
	m_ScaleSlider->SetCallback( staticScreenCB, this );
	m_ScaleSlider->SetLimits( -1000000.0, 1000000.0 );
	m_ScaleSlider->SetRange( 1.0 );
	m_ScaleSlider->UpdateGui();

	m_LowerLimitSlider = new SliderInputCombo( ui->lowerLimitSlider, ui->lowerLimitInput );
	m_LowerLimitSlider->SetCallback( staticScreenCB, this );
	m_LowerLimitSlider->SetLimits( -1.0e12, 1.0e12 );
	m_LowerLimitSlider->SetRange( 10.0 );
	m_LowerLimitSlider->UpdateGui();

	m_UpperLimitSlider = new SliderInputCombo( ui->upperLimitSlider, ui->upperLimitInput );
	m_UpperLimitSlider->SetCallback( staticScreenCB, this );
	m_UpperLimitSlider->SetLimits( -1.0e12, 1.0e12 );
	m_UpperLimitSlider->SetRange( 10.0 );
	m_UpperLimitSlider->UpdateGui();

	m_User1Slider = new Slider_adj_range(this, ui->user1Slider, ui->user1ButtonL, ui->user1ButtonR, 10.0 );
	m_User1Input = new Input( this, ui->user1Input );
	m_User2Slider = new Slider_adj_range(this, ui->user2Slider, ui->user2ButtonL, ui->user2ButtonR, 10.0 );
	m_User2Input = new Input( this, ui->user2Input );
	m_User3Slider = new Slider_adj_range(this, ui->user3Slider, ui->user3ButtonL, ui->user3ButtonR, 10.0 );
	m_User3Input = new Input( this, ui->user3Input );
	m_User4Slider = new Slider_adj_range(this, ui->user4Slider, ui->user4ButtonL, ui->user4ButtonR, 10.0 );
	m_User4Input = new Input( this, ui->user4Input );
	m_User5Slider = new Slider_adj_range(this, ui->user5Slider, ui->user5ButtonL, ui->user5ButtonR, 10.0 );
	m_User5Input = new Input( this, ui->user5Input );
	m_User6Slider = new Slider_adj_range(this, ui->user6Slider, ui->user6ButtonL, ui->user6ButtonR, 10.0 );
	m_User6Input = new Input( this, ui->user6Input );
	m_User7Slider = new Slider_adj_range(this, ui->user7Slider, ui->user7ButtonL, ui->user7ButtonR, 10.0 );
	m_User7Input = new Input( this, ui->user7Input );
	m_User8Slider = new Slider_adj_range(this, ui->user8Slider, ui->user8ButtonL, ui->user8ButtonR, 10.0 );
	m_User8Input = new Input( this, ui->user8Input );


	m_User1Button = new ParmButton( this, ui->user1Button );
	m_User2Button = new ParmButton( this, ui->user2Button );
	m_User3Button = new ParmButton( this, ui->user3Button );
	m_User4Button = new ParmButton( this, ui->user4Button );
	m_User5Button = new ParmButton( this, ui->user5Button );
	m_User6Button = new ParmButton( this, ui->user6Button );
	m_User7Button = new ParmButton( this, ui->user7Button );
	m_User8Button = new ParmButton( this, ui->user8Button );

}

ParmLinkScreen::~ParmLinkScreen()
{

}

void ParmLinkScreen::RemoveAllRefs( GeomBase* gPtr )
{
	vector< ParmButton* > tempVec;

	for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
	{
		Parm* p =  m_ParmButtonVec[i]->get_parm_ptr();
		if ( p && p->get_geom_base() != gPtr )
			tempVec.push_back( m_ParmButtonVec[i] );
	}
	m_ParmButtonVec = tempVec;
}

void ParmLinkScreen::update()
{
	int i;
	char str[256];

	ParmLink* currLink = parmLinkMgrPtr->GetCurrParmLink();

	if ( currLink->GetParmA() == NULL || currLink->GetParmB() == NULL )
		currLink = parmLinkMgrPtr->ResetWorkingParmLink();

	//==== Geom Names A ====//
	parmLinkUI->compAChoice->clear();
	vector< string > geomNameVecA;
	int indA = parmMgrPtr->GetCurrGeomNameVec( currLink->GetParmA(), geomNameVecA );
	for ( i = 0 ; i < (int)geomNameVecA.size() ; i++ )
	{
		sprintf( str, "%d-%s", i,  geomNameVecA[i].c_str() );
		parmLinkUI->compAChoice->add( str );
	}
		//parmLinkUI->compAChoice->add( geomNameVecA[i].c_str() );
	parmLinkUI->compAChoice->value( indA );

	//==== Group Names A ====//
	parmLinkUI->groupAChoice->clear();
	vector< string > groupNameVecA;
	indA = parmMgrPtr->GetCurrGroupNameVec( currLink->GetParmA(), groupNameVecA );
	for ( i = 0 ; i < (int)groupNameVecA.size() ; i++ )
		parmLinkUI->groupAChoice->add( groupNameVecA[i].c_str() );
	parmLinkUI->groupAChoice->value( indA );

	//==== Parm Names A =====//
	parmLinkUI->parmAChoice->clear();
	vector< string > parmNameVecA;
	indA = parmMgrPtr->GetCurrParmNameVec( currLink->GetParmA(), parmNameVecA );
	for ( i = 0 ; i < (int)parmNameVecA.size() ; i++ )
		parmLinkUI->parmAChoice->add( parmNameVecA[i].c_str() );
	parmLinkUI->parmAChoice->value( indA );

	//==== Geom Names B ====//
	parmLinkUI->compBChoice->clear();
	vector< string > geomNameVecB;
	int indB = parmMgrPtr->GetCurrGeomNameVec( currLink->GetParmB(), geomNameVecB );
	for ( i = 0 ; i < (int)geomNameVecB.size() ; i++ )
	{
		sprintf( str, "%d-%s", i,  geomNameVecB[i].c_str() );
		parmLinkUI->compBChoice->add( str );
	}
//		parmLinkUI->compBChoice->add( geomNameVecB[i].c_str() );
	parmLinkUI->compBChoice->value( indB );

	//==== Group Names B ====//
	parmLinkUI->groupBChoice->clear();
	vector< string > groupNameVecB;
	indB = parmMgrPtr->GetCurrGroupNameVec( currLink->GetParmB(), groupNameVecB );
	for ( i = 0 ; i < (int)groupNameVecB.size() ; i++ )
		parmLinkUI->groupBChoice->add( groupNameVecB[i].c_str() );
	parmLinkUI->groupBChoice->value( indB );

	//==== Parm Names B =====//
	parmLinkUI->parmBChoice->clear();
	vector< string > parmNameVecB;
	indB = parmMgrPtr->GetCurrParmNameVec( currLink->GetParmB(), parmNameVecB );
	for ( i = 0 ; i < (int)parmNameVecB.size() ; i++ )
		parmLinkUI->parmBChoice->add( parmNameVecB[i].c_str() );
	parmLinkUI->parmBChoice->value( indB );

	//===== Update Offset ====//
	parmLinkUI->offsetButton->value(1);
	m_OffsetSlider->Activate();
	if ( !currLink->GetOffsetFlag() )
	{
		parmLinkUI->offsetButton->value(0);
		m_OffsetSlider->Deactivate();
	}

	m_OffsetSlider->SetVal( currLink->GetOffset() );
	m_OffsetSlider->UpdateGui();

	//===== Update Scale ====//
	parmLinkUI->scaleButton->value(1);
	m_ScaleSlider->Activate();
	if ( !currLink->GetScaleFlag() )
	{
		parmLinkUI->scaleButton->value(0);
		m_ScaleSlider->Deactivate();
	}

	m_ScaleSlider->SetVal( currLink->GetScale() );
	m_ScaleSlider->UpdateGui();

	//===== Update Lower Limit ====//
	parmLinkUI->lowerLimitButton->value(1);
	m_LowerLimitSlider->Activate();
	if ( !currLink->GetLowerLimitFlag() )
	{
		parmLinkUI->lowerLimitButton->value(0);
		m_LowerLimitSlider->Deactivate();
	}
	m_LowerLimitSlider->SetVal( currLink->GetLowerLimit() );
	m_LowerLimitSlider->UpdateGui();

	//===== Update Upper Limit ====//
	parmLinkUI->upperLimitButton->value(1);
	m_UpperLimitSlider->Activate();
	if ( !currLink->GetUpperLimitFlag() )
	{
		parmLinkUI->upperLimitButton->value(0);
		m_UpperLimitSlider->Deactivate();
	}
	m_UpperLimitSlider->SetVal( currLink->GetUpperLimit() );
	m_UpperLimitSlider->UpdateGui();

	//==== Update Link Browser ====//
	parmLinkUI->linkBrowser->clear();

	static int widths[] = { 75,75,90,20,75,75,80,0 };	// widths for each column
	parmLinkUI->linkBrowser->column_widths(widths);		// assign array to widget
	parmLinkUI->linkBrowser->column_char(':');			// use : as the column character

	sprintf( str, "@b@.COMP_A:@b@.GROUP:@b@.PARM:->:@b@.COMP_B:@b@.GROUP:@b@.PARM" );
	parmLinkUI->linkBrowser->add( str ); 

	vector< ParmLink* > parmLinkVec = parmLinkMgrPtr->GetParmLinkVec();
	for ( i = 0 ; i < (int)parmLinkVec.size() ; i++ )
	{
		Parm* pa = parmLinkVec[i]->GetParmA();
		Parm* pb = parmLinkVec[i]->GetParmB();
		sprintf( str, "%s:%s:%s:->:%s:%s:%s", 
			pa->get_geom_base()->getName().get_char_star(), pa->get_group_name().get_char_star(), pa->get_name().get_char_star(),
			pb->get_geom_base()->getName().get_char_star(), pb->get_group_name().get_char_star(), pb->get_name().get_char_star() );
		parmLinkUI->linkBrowser->add( str ); 
	}

	int index = parmLinkMgrPtr->GetCurrParmLinkIndex();
	if ( index >= 0 && index < (int)parmLinkVec.size() ) 
		parmLinkUI->linkBrowser->select( index+2 );

	parmLinkUI->UIWindow->redraw();

	for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
	{
		m_ParmButtonVec[i]->update();
	}
}

void ParmLinkScreen::clearButtonParms()
{
	for ( int i = 0 ; i < (int)m_ParmButtonVec.size() ; i++ )
	{
		m_ParmButtonVec[i]->set_parm_ptr( 0 );
	}

}

void ParmLinkScreen::show()
{
	show( aircraftPtr->getUserGeom() );
}

void ParmLinkScreen::show( Geom* geomPtr )
{
	//==== Check For Duplicate Comp Names ====//



	UserGeom* currGeom = (UserGeom*)geomPtr;

	m_User1Slider->set_parm_ptr( &currGeom->userParm1 );
	m_User1Input->set_parm_ptr(  &currGeom->userParm1 );
	m_User2Slider->set_parm_ptr( &currGeom->userParm2 );
	m_User2Input->set_parm_ptr(  &currGeom->userParm2 );
	m_User3Slider->set_parm_ptr( &currGeom->userParm3 );
	m_User3Input->set_parm_ptr(  &currGeom->userParm3 );
	m_User4Slider->set_parm_ptr( &currGeom->userParm4 );
	m_User4Input->set_parm_ptr(  &currGeom->userParm4 );
	m_User5Slider->set_parm_ptr( &currGeom->userParm5 );
	m_User5Input->set_parm_ptr(  &currGeom->userParm5 );
	m_User6Slider->set_parm_ptr( &currGeom->userParm6 );
	m_User6Input->set_parm_ptr(  &currGeom->userParm6 );
	m_User7Slider->set_parm_ptr( &currGeom->userParm7 );
	m_User7Input->set_parm_ptr(  &currGeom->userParm7 );
	m_User8Slider->set_parm_ptr( &currGeom->userParm8 );
	m_User8Input->set_parm_ptr(  &currGeom->userParm8 );

	m_User1Button->set_parm_ptr( &currGeom->userParm1 );
	m_User2Button->set_parm_ptr( &currGeom->userParm2 );
	m_User3Button->set_parm_ptr( &currGeom->userParm3 );
	m_User4Button->set_parm_ptr( &currGeom->userParm4 );
	m_User5Button->set_parm_ptr( &currGeom->userParm5 );
	m_User6Button->set_parm_ptr( &currGeom->userParm6 );
	m_User7Button->set_parm_ptr( &currGeom->userParm7 );
	m_User8Button->set_parm_ptr( &currGeom->userParm8 );

	parmMgrPtr->LoadAllParms();
	update();
	parmLinkUI->UIWindow->show();
}

void ParmLinkScreen::hide()
{
	parmLinkUI->UIWindow->hide();
}

void ParmLinkScreen::setTitle( const char* name )
{
	//title = "PARMLINK : ";
	//title.concatenate( name );

	//parmLinkUI->TitleBox->label( title );
}

//==== Close Callbacks =====//
void ParmLinkScreen::closeCB( Fl_Widget* w)
{
	parmLinkUI->UIWindow->hide();
}


void ParmLinkScreen::screenCB( Fl_Widget* w )
{
	ParmLink* currLink = parmLinkMgrPtr->GetCurrParmLink();
	if ( w == parmLinkUI->compAChoice  || w == parmLinkUI->compBChoice ||
		 w == parmLinkUI->groupAChoice || w == parmLinkUI->groupBChoice ||
		 w == parmLinkUI->parmAChoice  || w == parmLinkUI->parmBChoice)
	{
		compGroupLinkChange();
	}
	else if (  w == parmLinkUI->offsetButton )
	{
		if ( parmLinkUI->offsetButton->value() )
			currLink->SetOffsetFlag( true );
		else
			currLink->SetOffsetFlag( false );

		m_OffsetSlider->Activate();
		if ( !currLink->GetOffsetFlag() )
			m_OffsetSlider->Deactivate();
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if (  w == parmLinkUI->scaleButton )
	{
		if ( parmLinkUI->scaleButton->value() )
			currLink->SetScaleFlag( true );
		else
			currLink->SetScaleFlag( false );

		m_ScaleSlider->Activate();
		if ( !currLink->GetScaleFlag() )
			m_ScaleSlider->Deactivate();
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if (  w == parmLinkUI->lowerLimitButton )
	{
		if ( parmLinkUI->lowerLimitButton->value() )
			currLink->SetLowerLimitFlag( true );
		else
			currLink->SetLowerLimitFlag( false );

		m_LowerLimitSlider->Activate();
		if ( !currLink->GetLowerLimitFlag() )
			m_LowerLimitSlider->Deactivate();
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if (  w == parmLinkUI->upperLimitButton )
	{
		if ( parmLinkUI->upperLimitButton->value() )
			currLink->SetUpperLimitFlag( true );
		else
			currLink->SetUpperLimitFlag( false );

		m_UpperLimitSlider->Activate();
		if ( !currLink->GetUpperLimitFlag() )
			m_UpperLimitSlider->Deactivate();
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if (  w == parmLinkUI->addLinkButton )
	{
		bool success = parmLinkMgrPtr->AddCurrLink();
		if ( !success )
			fl_alert( "Error: Identical Parms or Already Linked" );
		update();
	}
	else if (  w == parmLinkUI->deleteLinkButton )
	{
		parmLinkMgrPtr->DelCurrLink();
		update();
	}
	else if (  w == parmLinkUI->deleteAllLinksButton )
	{
		parmLinkMgrPtr->DelAllLinks();
		update();
	}
	else if (  w == parmLinkUI->linkAllCompButton )
	{
		bool success = parmLinkMgrPtr->LinkAllComp();
		if ( !success )
			fl_alert( "Error: Identical Comps" );
		update();
	}
	else if (  w == parmLinkUI->linkAllGroupButton )
	{
		bool success = parmLinkMgrPtr->LinkAllGroup();
		if ( !success )
			fl_alert( "Error: Identical Group" );
		update();
	}
	else if (  w == parmLinkUI->linkBrowser )
	{
		int sel = parmLinkUI->linkBrowser->value();
		parmLinkMgrPtr->SetCurrParmLinkIndex( sel-2 );	
		update();
	}
	else if ( m_OffsetSlider->GuiChanged( w ) )
	{
		currLink->SetOffset( m_OffsetSlider->GetVal() );
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if ( m_ScaleSlider->GuiChanged( w ) )
	{
		currLink->SetScale( m_ScaleSlider->GetVal() );
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if ( m_LowerLimitSlider->GuiChanged( w ) )
	{
		currLink->SetLowerLimit( m_LowerLimitSlider->GetVal() );
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}
	else if ( m_UpperLimitSlider->GuiChanged( w ) )
	{
		currLink->SetUpperLimit( m_UpperLimitSlider->GetVal() );
		parmLinkMgrPtr->ParmChanged( currLink->GetParmA(), true );
	}

	aircraftPtr->triggerDraw();

}



void ParmLinkScreen::compGroupLinkChange()
{
	ParmLinkUI* ui = parmLinkUI;
	parmLinkMgrPtr->SetCurrParmLinkIndex(-1);
	parmLinkMgrPtr->SetParm( true,
		ui->compAChoice->value(), ui->groupAChoice->value(), ui->parmAChoice->value() );
	parmLinkMgrPtr->SetParm( false,
		ui->compBChoice->value(), ui->groupBChoice->value(), ui->parmBChoice->value() );
	update();
}

