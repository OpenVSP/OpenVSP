//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "DegenGeomScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include <assert.h>

DegenGeomScreen::DegenGeomScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    DegenGeomUI* ui = m_DegenGeomUI = new DegenGeomUI();

    m_FLTK_Window = ui->UIWindow;
    m_textBuffer = new Fl_Text_Buffer();

    ui->setChoice->callback( staticScreenCB, this );
    ui->csvFileButton->callback( staticScreenCB, this );
    ui->csvFileChooseButton->callback( staticScreenCB, this );
    ui->computeButton->callback( staticScreenCB, this );
    ui->outputTextDisplay->callback( staticScreenCB, this );
    ui->mFileButton->callback( staticScreenCB, this );
    ui->mFileChooseButton->callback( staticScreenCB, this );
    ui->outputTextDisplay->buffer( m_textBuffer );

    m_SelectedSetIndex = 0;
}

bool DegenGeomScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();
    LoadSetChoice();

    if ( vehiclePtr->getExportDegenGeomCsvFile() )
    {
    	m_DegenGeomUI->csvFileButton->value( 1 );
    }
    else
    {
    	m_DegenGeomUI->csvFileButton->value( 0 );
    }

    if ( vehiclePtr->getExportDegenGeomMFile() )
    {
    	m_DegenGeomUI->mFileButton->value( 1 );
    }
    else
    {
    	m_DegenGeomUI->mFileButton->value( 0 );
    }

    m_DegenGeomUI->csvFileOutput->value( vehiclePtr->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE ).c_str() );
    m_DegenGeomUI->mFileOutput->value( vehiclePtr->getExportFileName( vsp::DEGEN_GEOM_M_TYPE ).c_str() );

    return true;
}

void DegenGeomScreen::LoadSetChoice()
{
	m_DegenGeomUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
    	m_DegenGeomUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_DegenGeomUI->setChoice->value( m_SelectedSetIndex );
}

DegenGeomScreen::~DegenGeomScreen()
{
    delete m_textBuffer;
}

void DegenGeomScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void DegenGeomScreen::Hide()
{
    m_FLTK_Window->hide();
}

void DegenGeomScreen::CallBack( Fl_Widget *w )
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

	if ( w == m_DegenGeomUI->csvFileButton )
	{
		vehiclePtr->setExportDegenGeomCsvFile( !!m_DegenGeomUI->csvFileButton->value() );
	}
	else if ( w == m_DegenGeomUI->csvFileChooseButton )
	{
        vehiclePtr->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select degen geom CSV output file.", "*.csv" ) );
	}
	else if ( w == m_DegenGeomUI->mFileButton )
	{
		vehiclePtr->setExportDegenGeomMFile( !!m_DegenGeomUI->mFileButton->value() );
	}
	else if ( w == m_DegenGeomUI->mFileChooseButton )
	{
        vehiclePtr->setExportFileName( vsp::DEGEN_GEOM_M_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select degen geom Matlab output file.", "*.m" ) );
	}
    else if ( w == m_DegenGeomUI->setChoice )
    {
        m_SelectedSetIndex = m_DegenGeomUI->setChoice->value();
    }
	else if (w == m_DegenGeomUI->computeButton)
	{
		m_DegenGeomUI->outputTextDisplay->buffer()->text("");
		m_DegenGeomUI->outputTextDisplay->buffer()->append("Computing degenerate geometry...\n");
		Fl::flush();
		vehiclePtr->CreateDegenGeom( m_SelectedSetIndex );
		m_DegenGeomUI->outputTextDisplay->buffer()->append("Done!\n");
		if ( vehiclePtr->getExportDegenGeomCsvFile() || vehiclePtr->getExportDegenGeomMFile() )
		{
			m_DegenGeomUI->outputTextDisplay->buffer()->append("--------------------------------\n");
			m_DegenGeomUI->outputTextDisplay->buffer()->append("\nWriting output...\n");
			Fl::flush();
			m_DegenGeomUI->outputTextDisplay->buffer()->append( vehiclePtr->WriteDegenGeomFile().c_str() );
		}
	}

    m_ScreenMgr->SetUpdateFlag( true );
}


