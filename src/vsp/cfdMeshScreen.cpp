// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "cfdMeshScreen.h"
#include "GridDensity.h"
#include "groupScreen.h"
#include "scriptMgr.h"
#include "CfdMeshMgr.h"
#include <FL/Fl_File_Chooser.H>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CfdMeshScreen::CfdMeshScreen(ScreenMgr* mgr, Aircraft* airPtr)
{
	screenMgrPtr = mgr;
	aircraftPtr = airPtr;
	currSourceType = -1;

	CFDMeshUI* ui = cfdMeshUI = new CFDMeshUI();
	cfdMeshUI->UIWindow->position( 760, 30 );

	//ui->intersectMeshButton->callback( staticScreenCB, this );
	//ui->meshSingleButton->callback( staticScreenCB, this );
	//ui->meshAllButton->callback( staticScreenCB, this );
	ui->finalMeshButton->callback( staticScreenCB, this );
	ui->viewMeshButton->callback( staticScreenCB, this );
	ui->viewSourceButton->callback( staticScreenCB, this );
	ui->halfMeshButton->callback( staticScreenCB, this );

	ui->SourceNameInput->callback( staticScreenCB, this );

	m_LengthSlider = new SliderInputCombo( ui->lengthSlider, ui->lengthInput );
	m_LengthSlider->SetCallback( staticScreenCB, this );
	m_LengthSlider->SetLimits( 0.0001, 1000000.0 );
	m_LengthSlider->SetRange( 1.0 );
	m_LengthSlider->UpdateGui();

	m_RadiusSlider = new SliderInputCombo( ui->radiusSlider, ui->radiusInput );
	m_RadiusSlider->SetCallback( staticScreenCB, this );
	m_RadiusSlider->SetLimits( 0.0001, 1000000.0 );
	m_RadiusSlider->SetRange( 1.0 );
	m_RadiusSlider->UpdateGui();

	m_Length2Slider = new SliderInputCombo( ui->length2Slider, ui->length2Input );
	m_Length2Slider->SetCallback( staticScreenCB, this );
	m_Length2Slider->SetLimits( 0.0001, 1000000.0 );
	m_Length2Slider->SetRange( 1.0 );
	m_Length2Slider->UpdateGui();

	m_Radius2Slider = new SliderInputCombo( ui->radius2Slider, ui->radius2Input );
	m_Radius2Slider->SetCallback( staticScreenCB, this );
	m_Radius2Slider->SetLimits( 0.0001, 1000000.0 );
	m_Radius2Slider->SetRange( 1.0 );
	m_Radius2Slider->UpdateGui();

	m_GlobalEdgeSizeSlider = new SliderInputCombo( ui->globalEdgeSizeSlider, ui->globalEdgeSizeInput );
	m_GlobalEdgeSizeSlider->SetCallback( staticScreenCB, this );
	m_GlobalEdgeSizeSlider->SetLimits( 0.0001, 1000000.0 );
	m_GlobalEdgeSizeSlider->SetRange( 1.0 );
	m_GlobalEdgeSizeSlider->UpdateGui();

	m_FarXScaleSlider = new SliderInputCombo( ui->farXSlider, ui->farXInput );
	m_FarXScaleSlider->SetCallback( staticScreenCB, this );
	m_FarXScaleSlider->SetLimits( 1.1, 10000.0 );
	m_FarXScaleSlider->SetRange( 10.0 );
	m_FarXScaleSlider->UpdateGui();

	m_FarYScaleSlider = new SliderInputCombo( ui->farYSlider, ui->farYInput );
	m_FarYScaleSlider->SetCallback( staticScreenCB, this );
	m_FarYScaleSlider->SetLimits( 1.1, 10000.0 );
	m_FarYScaleSlider->SetRange( 10.0 );
	m_FarYScaleSlider->UpdateGui();

	m_FarZScaleSlider = new SliderInputCombo( ui->farZSlider, ui->farZInput );
	m_FarZScaleSlider->SetCallback( staticScreenCB, this );
	m_FarZScaleSlider->SetLimits( 1.1, 10000.0 );
	m_FarZScaleSlider->SetRange( 10.0 );
	m_FarZScaleSlider->UpdateGui();

	ui->compChoice->callback( staticScreenCB, this );
	ui->sourceBrowser->callback( staticScreenCB, this );

	ui->u1Slider->callback( staticScreenCB, this );
	ui->w1Slider->callback( staticScreenCB, this );
	ui->u1Input->callback( staticScreenCB, this );
	ui->w1Input->callback( staticScreenCB, this );

	ui->u2Slider->callback( staticScreenCB, this );
	ui->w2Slider->callback( staticScreenCB, this );
	ui->u2Input->callback( staticScreenCB, this );
	ui->w2Input->callback( staticScreenCB, this );

	ui->addSourceButton->callback( staticScreenCB, this );
	ui->deleteSourceButton->callback( staticScreenCB, this );

	ui->addDefaultsButton->callback( staticScreenCB, this );

	ui->adjLenDownButton->callback( staticScreenCB, this );
	ui->adjLenDownDownButton->callback( staticScreenCB, this );
	ui->adjLenUpButton->callback( staticScreenCB, this );
	ui->adjLenUpUpButton->callback( staticScreenCB, this );
	ui->adjRadDownButton->callback( staticScreenCB, this );
	ui->adjRadDownDownButton->callback( staticScreenCB, this );
	ui->adjRadUpButton->callback( staticScreenCB, this );
	ui->adjRadUpUpButton->callback( staticScreenCB, this );

	ui->outputText->buffer( &m_TextBuffer );

	ui->datButton->callback( staticScreenCB, this );
	ui->keyButton->callback( staticScreenCB, this );
	ui->objButton->callback( staticScreenCB, this );
	ui->polyButton->callback( staticScreenCB, this );
	ui->stlButton->callback( staticScreenCB, this );
	ui->triButton->callback( staticScreenCB, this );
	ui->gmshButton->callback( staticScreenCB, this );
	ui->srfButton->callback( staticScreenCB, this );
	ui->datToggle->value(0);
	ui->keyToggle->value(0);
	ui->objToggle->value(1);
	ui->polyToggle->value(0);
	ui->stlToggle->value(1);
	ui->triToggle->value(1);
	ui->gmshToggle->value(0);
	ui->srfToggle->value(0);

}

CfdMeshScreen::~CfdMeshScreen()
{
	delete m_LengthSlider;
	delete m_RadiusSlider;
	delete m_Length2Slider;
	delete m_Radius2Slider;
	delete m_GlobalEdgeSizeSlider;
	delete m_FarXScaleSlider;
	delete m_FarYScaleSlider;
	delete m_FarZScaleSlider;

	delete cfdMeshUI;
}

void CfdMeshScreen::update()
{
	int i;
	char str[256];

	//==== Base Len ====//
	m_GlobalEdgeSizeSlider->SetVal(cfdMeshMgrPtr->GetGridDensityPtr()->GetBaseLen()); 
	m_GlobalEdgeSizeSlider->UpdateGui();

	m_FarXScaleSlider->SetVal( cfdMeshMgrPtr->GetFarXScale() );
	m_FarYScaleSlider->SetVal( cfdMeshMgrPtr->GetFarYScale() );
	m_FarZScaleSlider->SetVal( cfdMeshMgrPtr->GetFarZScale() );
	m_FarXScaleSlider->UpdateGui();
	m_FarYScaleSlider->UpdateGui();
	m_FarZScaleSlider->UpdateGui();

	//==== Load Geom Choice ====//
	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	cfdMeshUI->compChoice->clear();
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		cfdMeshUI->compChoice->add( geomVec[i]->getName() );
	}

	int currGeomID = cfdMeshMgrPtr->GetCurrGeomID();
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		cfdMeshUI->compChoice->value( currGeomID );
	}

	BaseSource* source = cfdMeshMgrPtr->GetCurrSource();

	if ( source )
	{
		m_LengthSlider->SetVal( source->GetLen() );
		m_LengthSlider->UpdateGui();

		m_RadiusSlider->SetVal( source->GetRad() );
		m_RadiusSlider->UpdateGui();

		cfdMeshUI->SourceNameInput->value( source->GetName() );
		//sprintf(str, "  %5.4f", source->GetLen() );
		//cfdMeshUI->lengthInput->value( str );
		//sprintf(str, "  %5.4f", source->GetRad() );
		//cfdMeshUI->radiusInput->value( str );

		if ( source->GetType() == BaseSource::POINT_SOURCE )
		{
			PointSource* ps = (PointSource*)source;
			vec2d uw = ps->GetUWLoc();
			cfdMeshUI->u1Slider->value( uw.x() );
			cfdMeshUI->w1Slider->value( uw.y() );
			sprintf( str, "%5.4f", uw.x() );	cfdMeshUI->u1Input->value( str );
			sprintf( str, "%5.4f", uw.y() );	cfdMeshUI->w1Input->value( str );
			cfdMeshUI->EditSourceTitle->label( "Edit Point Source" );
		}
		else if ( source->GetType() == BaseSource::LINE_SOURCE )
		{
			LineSource* ps = (LineSource*)source;
			vec2d uw1 = ps->GetUWLoc1();
			cfdMeshUI->u1Slider->value( uw1.x() );
			cfdMeshUI->w1Slider->value( uw1.y() );
			sprintf( str, "%5.4f", uw1.x() );	cfdMeshUI->u1Input->value( str );
			sprintf( str, "%5.4f", uw1.y() );	cfdMeshUI->w1Input->value( str );
			vec2d uw2 = ps->GetUWLoc2();
			cfdMeshUI->u2Slider->value( uw2.x() );
			cfdMeshUI->w2Slider->value( uw2.y() );
			sprintf( str, "%5.4f", uw2.x() );	cfdMeshUI->u2Input->value( str );
			sprintf( str, "%5.4f", uw2.y() );	cfdMeshUI->w2Input->value( str );

			m_Length2Slider->SetVal( ps->GetLen2() );
			m_Length2Slider->UpdateGui();

			m_Radius2Slider->SetVal( ps->GetRad2() );
			m_Radius2Slider->UpdateGui();

			cfdMeshUI->EditSourceTitle->label( "Edit Line Source" );
		}
		else if ( source->GetType() == BaseSource::BOX_SOURCE )
		{
			BoxSource* ps = (BoxSource*)source;
			vec2d uw1 = ps->GetUWLoc1();
			cfdMeshUI->u1Slider->value( uw1.x() );
			cfdMeshUI->w1Slider->value( uw1.y() );
			sprintf( str, "%5.4f", uw1.x() );	cfdMeshUI->u1Input->value( str );
			sprintf( str, "%5.4f", uw1.y() );	cfdMeshUI->w1Input->value( str );
			vec2d uw2 = ps->GetUWLoc2();
			cfdMeshUI->u2Slider->value( uw2.x() );
			cfdMeshUI->w2Slider->value( uw2.y() );
			sprintf( str, "%5.4f", uw2.x() );	cfdMeshUI->u2Input->value( str );
			sprintf( str, "%5.4f", uw2.y() );	cfdMeshUI->w2Input->value( str );
			cfdMeshUI->EditSourceTitle->label( "Edit Box Source" );
		}
	}

	//==== Load Up Source Browser ====//
	int currSourceID = -1;
	if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
	{
		cfdMeshUI->sourceBrowser->clear();
		vector< BaseSource* > sVec = geomVec[currGeomID]->getCfdMeshSourceVec();
		for ( i = 0 ; i < (int)sVec.size() ; i++ )
		{
			if ( source == sVec[i] )
				currSourceID = i;

			cfdMeshUI->sourceBrowser->add( sVec[i]->GetName() );
		}

		if ( currSourceID >= 0 && currSourceID < (int)sVec.size() )
			cfdMeshUI->sourceBrowser->select( currSourceID+1 );

	}

	//==== Show/Hide UI Depending on Part ====//
	int type = -1;
	if ( source )
	{
		type = source->GetType();
	}

	if ( type != currSourceType )
	{
		currSourceType = type;

		if ( currSourceType == BaseSource::POINT_SOURCE )
		{
			cfdMeshUI->sourceEditGroup->hide();
			cfdMeshUI->lineRadLenGroup->hide();
		}
		else if ( currSourceType == BaseSource::LINE_SOURCE )
		{
			cfdMeshUI->sourceEditGroup->show();
			cfdMeshUI->lineRadLenGroup->show();
		}
		else
		{
			cfdMeshUI->sourceEditGroup->show();
			cfdMeshUI->lineRadLenGroup->hide();
		}
	}

	if ( cfdMeshMgrPtr->GetDrawSourceFlag() )
		cfdMeshUI->viewSourceButton->value(1);
	else
		cfdMeshUI->viewSourceButton->value(0);

	if ( cfdMeshMgrPtr->GetHalfMeshFlag() )
		cfdMeshUI->halfMeshButton->value(1);
	else
		cfdMeshUI->halfMeshButton->value(0);

	Stringc datname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::DAT_FILE_NAME );
	cfdMeshUI->datName->value( truncateFileName(datname, 40 ) );
	Stringc keyname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::KEY_FILE_NAME );
	cfdMeshUI->keyName->value( truncateFileName(keyname, 40 ) );
	Stringc objname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::OBJ_FILE_NAME );
	cfdMeshUI->objName->value( truncateFileName(objname, 40 ) );
	Stringc polyname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::POLY_FILE_NAME );
	cfdMeshUI->polyName->value( truncateFileName(polyname, 40 ) );
	Stringc stlname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::STL_FILE_NAME );
	cfdMeshUI->stlName->value( truncateFileName(stlname, 40 ) );
	Stringc triname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::TRI_FILE_NAME );
	cfdMeshUI->triName->value( truncateFileName(triname, 40 ) );
	Stringc gmshname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::GMSH_FILE_NAME );
	cfdMeshUI->gmshName->value( truncateFileName(gmshname, 40 ) );
	Stringc srfname = cfdMeshMgrPtr->GetExportFileName( CfdMeshMgr::SRF_FILE_NAME );
	cfdMeshUI->srfName->value( truncateFileName(srfname, 40 ) );

}

Stringc CfdMeshScreen::truncateFileName( const char* fn, int len )
{
	Stringc trunc( fn );
	if ( trunc.get_length() > len )
	{
		trunc.delete_range( 0, trunc.get_length()-len );
		trunc.overwrite_at_position(0, "...");
	}
	return trunc;
}

void CfdMeshScreen::setMeshExportFlags()
{
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->stlToggle->value(), CfdMeshMgr::STL_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->polyToggle->value(), CfdMeshMgr::POLY_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->triToggle->value(), CfdMeshMgr::TRI_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->objToggle->value(), CfdMeshMgr::OBJ_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->datToggle->value(), CfdMeshMgr::DAT_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->keyToggle->value(), CfdMeshMgr::KEY_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->gmshToggle->value(), CfdMeshMgr::GMSH_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!cfdMeshUI->srfToggle->value(), CfdMeshMgr::SRF_FILE_NAME );
}

void CfdMeshScreen::show()
{
	update();
	cfdMeshUI->UIWindow->show();
}

void CfdMeshScreen::addOutputText( const char* text )
{
		m_TextBuffer.append( text );
		cfdMeshUI->outputText->move_down();
		cfdMeshUI->outputText->show_insert_position();
		Fl::flush();
}

void CfdMeshScreen::screenCB( Fl_Widget* w )
{
	static bool intersectFlag = false;
	bool update_flag = true;

	//if ( w == cfdMeshUI->intersectMeshButton  )
	//{
	//	intersectFlag = true;
	//	addOutputText( "Writing Bezier File\n" );
	//	aircraftPtr->write_bezier_file( "cfdmesh.bez" );

	//	cfdMeshMgrPtr->CleanUp();
	//	addOutputText( "Reading Surfaces\n");
	//	cfdMeshMgrPtr->ReadSurfs("cfdmesh.bez");

	//	cfdMeshMgrPtr->UpdateSources();
	//	addOutputText( "Build Grid\n");
	//	cfdMeshMgrPtr->BuildGrid();
	//	addOutputText( "Intersect\n");
	//	cfdMeshMgrPtr->Intersect();
	//	addOutputText( "Finished Intersect\n");


	//}
	//else if ( w == cfdMeshUI->meshSingleButton  )
	//{

	//	if ( !intersectFlag )
	//		fl_alert("Error: Press Intersect First");

	//	cfdMeshMgrPtr->UpdateSources();
	//	addOutputText( "InitMesh\n");
	//	cfdMeshMgrPtr->InitMesh();
	//	addOutputText( "Remesh\n");

	//	int comp_id = 0;
	//	int currGeomID = cfdMeshMgrPtr->GetCurrGeomID();
	//	cfdMeshMgrPtr->RemeshSingleComp( currGeomID, CfdMeshMgr::CFD_OUTPUT );
	//	addOutputText( "Write cfdmesh.stl\n");
	//	cfdMeshMgrPtr->WriteSTL("cfdmesh.stl");

	//	//==== No Show Components ====//
	//	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	//	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	//		geomVec[i]->setNoShowFlag(1);
	//	screenMgrPtr->update( GEOM_SCREEN );

	//	cfdMeshMgrPtr->SetDrawMeshFlag( true );
	//	cfdMeshUI->viewMeshButton->value(1);
	//}
	//else if ( w == cfdMeshUI->meshAllButton  )
	//{
	//	if ( !intersectFlag )
	//		fl_alert("Error: Press Intersect First");

	//	cfdMeshMgrPtr->UpdateSources();
	//	addOutputText( "InitMesh\n");
	//	cfdMeshMgrPtr->InitMesh();
	//	addOutputText( "Remesh\n");
	//	cfdMeshMgrPtr->Remesh( CfdMeshMgr::CFD_OUTPUT );
	//	addOutputText( "Mesh Complete\n");

	//	//==== No Show Components ====//
	//	vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
	//	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	//		geomVec[i]->setNoShowFlag(1);
	//	screenMgrPtr->update( GEOM_SCREEN );
	//	cfdMeshMgrPtr->SetDrawMeshFlag( true );
	//	cfdMeshUI->viewMeshButton->value(1);
	//}
	if ( w == cfdMeshUI->viewMeshButton )
	{
		if ( cfdMeshUI->viewMeshButton->value() )
			cfdMeshMgrPtr->SetDrawMeshFlag( true );
		else
			cfdMeshMgrPtr->SetDrawMeshFlag( false );
	}
	else if ( w == cfdMeshUI->viewSourceButton )
	{
		if ( cfdMeshUI->viewSourceButton->value() )
			cfdMeshMgrPtr->SetDrawSourceFlag( true );
		else
			cfdMeshMgrPtr->SetDrawSourceFlag( false );
	}
	else if ( w == cfdMeshUI->halfMeshButton )
	{
		if ( cfdMeshUI->halfMeshButton->value() )
			cfdMeshMgrPtr->SetHalfMeshFlag( true );
		else
			cfdMeshMgrPtr->SetHalfMeshFlag( false );
	}
	else if ( w == cfdMeshUI->finalMeshButton )
	{
		addOutputText( "Writing Bezier File\n" );
		Stringc bezTempFile = aircraftPtr->getTempDir();
		bezTempFile.concatenate( Stringc( "cfdmesh.bez" ) );
		cfdMeshMgrPtr->WriteSurfs( bezTempFile );

		cfdMeshMgrPtr->CleanUp();
		addOutputText( "Reading Surfaces\n");
		cfdMeshMgrPtr->ReadSurfs( bezTempFile );

		cfdMeshMgrPtr->UpdateSources();
		addOutputText( "Build Grid\n");
		cfdMeshMgrPtr->BuildGrid();
		addOutputText( "Intersect\n");
		cfdMeshMgrPtr->Intersect();
		addOutputText( "Finished Intersect\n");
		cfdMeshMgrPtr->UpdateSources();
		addOutputText( "InitMesh\n");
		cfdMeshMgrPtr->InitMesh();
		addOutputText( "Remesh\n");
		cfdMeshMgrPtr->Remesh( CfdMeshMgr::CFD_OUTPUT );
		//addOutputText( "Triangle Quality\n");
		//Stringc qual = cfdMeshMgrPtr->GetQualString();
		//addOutputText( qual.get_char_star() );
		addOutputText( "Exporting Files\n");
		setMeshExportFlags();
		cfdMeshMgrPtr->ExportFiles();
		//addOutputText( "Write STL: cfdmesh.stl\n");
		//cfdMeshMgrPtr->WriteSTL("cfdmesh.stl");
		//addOutputText( "Write NASCART: bodyin.dat bodyin.key\n");
		//cfdMeshMgrPtr->WriteNASCART("bodyin.dat", "bodyin.key" );
		//addOutputText( "Write TetGen: tetgen.poly\n");
		//cfdMeshMgrPtr->WriteTetGen("tetgen.poly");

		addOutputText( "Check Water Tight\n");
		Stringc resultTxt = cfdMeshMgrPtr->CheckWaterTight();
		addOutputText( resultTxt.get_char_star() );
		
//		addOutputText( "Mesh Complete\n");

		//==== No Show Components ====//
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();	
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
			geomVec[i]->setNoShowFlag(1);
		screenMgrPtr->update( GEOM_SCREEN );
		cfdMeshMgrPtr->SetDrawMeshFlag( true );
		cfdMeshUI->viewMeshButton->value(1);
		//aircraftPtr->triggerDraw();
	}
	else if ( w == cfdMeshUI->addDefaultsButton )
	{
		vector< Geom* > geomVec = aircraftPtr->getGeomVec();	

		int currGeomID = cfdMeshMgrPtr->GetCurrGeomID();
		if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
		{
			double base_len = cfdMeshMgrPtr->GetGridDensityPtr()->GetBaseLen();
			geomVec[currGeomID]->AddDefaultSources(base_len);
		}
	}
	else if ( m_GlobalEdgeSizeSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->GUI_Val( "GlobalEdgeSize", m_GlobalEdgeSizeSlider->GetVal() );
		update_flag = false;
	}
	else if ( m_FarXScaleSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->SetFarXScale( m_FarXScaleSlider->GetVal() );
		update_flag = false;
	}
	else if ( m_FarYScaleSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->SetFarYScale( m_FarYScaleSlider->GetVal() );
		update_flag = false;
	}
	else if ( m_FarZScaleSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->SetFarZScale( m_FarZScaleSlider->GetVal() );
		update_flag = false;
	}
	//else if ( w == cfdMeshUI->globalEdgeSizeInput )
	//{
	//	cfdMeshMgrPtr->GUI_Val( "GlobalEdgeSize", atof( cfdMeshUI->globalEdgeSizeInput->value() ) );
	//}
	else if ( w == cfdMeshUI->SourceNameInput )
	{
		cfdMeshMgrPtr->GUI_Val( "SourceName", Stringc(cfdMeshUI->SourceNameInput->value()) );
	}
	else if ( m_LengthSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->GUI_Val( "Length1", m_LengthSlider->GetVal() );
		update_flag = false;
	}
	else if ( m_RadiusSlider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->GUI_Val( "Radius1", m_RadiusSlider->GetVal() );
		update_flag = false;
	}
	else if ( m_Length2Slider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->GUI_Val( "Length2", m_Length2Slider->GetVal() );
		update_flag = false;
	}
	else if ( m_Radius2Slider->GuiChanged( w ) )
	{
		cfdMeshMgrPtr->GUI_Val( "Radius2", m_Radius2Slider->GetVal() );
		update_flag = false;
	}
	//else if ( w == cfdMeshUI->lengthInput )
	//{
	//	cfdMeshMgrPtr->GUI_Val( "Length", atof( cfdMeshUI->lengthInput->value() ) );
	//}
	//else if ( w == cfdMeshUI->radiusInput )
	//{
	//	cfdMeshMgrPtr->GUI_Val( "Radius", atof( cfdMeshUI->radiusInput->value() ) );
	//}
	else if ( w == cfdMeshUI->compChoice )
	{
		//==== Load List of Parts for Comp ====//
		int id = cfdMeshUI->compChoice->value();
		cfdMeshMgrPtr->SetCurrGeomID( id );
	}
	else if ( w == cfdMeshUI->sourceBrowser )
	{
		cfdMeshMgrPtr->GUI_Val( "SourceID", cfdMeshUI->sourceBrowser->value()-1 );
	}

	else if ( w == cfdMeshUI->u1Slider )
		cfdMeshMgrPtr->GUI_Val( "U1", cfdMeshUI->u1Slider->value() );
	else if ( w == cfdMeshUI->w1Slider )
		cfdMeshMgrPtr->GUI_Val( "W1", cfdMeshUI->w1Slider->value() );
	else if ( w == cfdMeshUI->u1Input )
		cfdMeshMgrPtr->GUI_Val( "U1", atof( cfdMeshUI->u1Input->value() ) );
	else if ( w == cfdMeshUI->w1Input )
		cfdMeshMgrPtr->GUI_Val( "W1", atof( cfdMeshUI->w1Input->value() ) );

	else if ( w == cfdMeshUI->u2Slider )
		cfdMeshMgrPtr->GUI_Val( "U2", cfdMeshUI->u2Slider->value() );
	else if ( w == cfdMeshUI->w2Slider )
		cfdMeshMgrPtr->GUI_Val( "W2", cfdMeshUI->w2Slider->value() );
	else if ( w == cfdMeshUI->u2Input )
		cfdMeshMgrPtr->GUI_Val( "U2", atof( cfdMeshUI->u2Input->value() ) );
	else if ( w == cfdMeshUI->w2Input )
		cfdMeshMgrPtr->GUI_Val( "W2", atof( cfdMeshUI->w2Input->value() ) );

	else if ( w == cfdMeshUI->addSourceButton )
	{
		int type = cfdMeshUI->sourceTypeChoice->value();
		if ( type >= 0 && type < BaseSource::NUM_SOURCE_TYPES )
			cfdMeshMgrPtr->AddSource( type );
	}
	else if ( w == cfdMeshUI->deleteSourceButton )
	{
		cfdMeshMgrPtr->DeleteCurrSource();
	}
	else if ( w == cfdMeshUI->adjLenDownButton )
		cfdMeshMgrPtr->AdjustAllSourceLen( 1.0/1.1 );
	else if ( w == cfdMeshUI->adjLenUpButton )
		cfdMeshMgrPtr->AdjustAllSourceLen( 1.1 );
	else if ( w == cfdMeshUI->adjLenDownDownButton )
		cfdMeshMgrPtr->AdjustAllSourceLen( 1.0/1.5 );
	else if ( w == cfdMeshUI->adjLenUpUpButton )
		cfdMeshMgrPtr->AdjustAllSourceLen( 1.5 );
	else if ( w == cfdMeshUI->adjRadDownButton )
		cfdMeshMgrPtr->AdjustAllSourceRad( 1.0/1.1 );
	else if ( w == cfdMeshUI->adjRadUpButton )
		cfdMeshMgrPtr->AdjustAllSourceRad( 1.1 );
	else if ( w == cfdMeshUI->adjRadDownDownButton )
		cfdMeshMgrPtr->AdjustAllSourceRad( 1.0/1.5 );
	else if ( w == cfdMeshUI->adjRadUpUpButton )
		cfdMeshMgrPtr->AdjustAllSourceRad( 1.5 );

	else if ( w == cfdMeshUI->datButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select NASCART .dat file.", "*.dat");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::DAT_FILE_NAME );
	}
	else if ( w == cfdMeshUI->keyButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select NASCART .key file.", "*.key");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::KEY_FILE_NAME );
	}
	else if ( w == cfdMeshUI->objButton  )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .obj file.", "*.obj");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::OBJ_FILE_NAME );
	}
	else if ( w == cfdMeshUI->polyButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .poly file.", "*.poly");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::POLY_FILE_NAME );
	}
	else if ( w == cfdMeshUI->stlButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .stl file.", "*.stl");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::STL_FILE_NAME );
	}
	else if ( w == cfdMeshUI->triButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .tri file.", "*.tri");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::TRI_FILE_NAME );
	}
	else if ( w == cfdMeshUI->gmshButton )
	{
		char *newfile = screenMgrPtr->FileChooser("Select .msh file.", "*.msh");
		if ( newfile != NULL )
			cfdMeshMgrPtr->SetExportFileName( newfile, CfdMeshMgr::GMSH_FILE_NAME );
	}


	if ( update_flag )
		update();

	aircraftPtr->triggerDraw();

}

